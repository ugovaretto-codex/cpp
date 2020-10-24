// Author: Ugo Varetto
// buffer allocation performance tests: vector, vector+pod allocator (same as
// vector), raw aligned buffer (faster)

#include <sys/mman.h>
#include <unistd.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;
using Clock = chrono::high_resolution_clock;
template <typename TimeDiffNsT>
constexpr double NsToSec(const TimeDiffNsT& diff) {
    return double(chrono::duration_cast<chrono::nanoseconds>(diff).count()) /
           1E9;
}

double now() { return clock() / double(CLOCKS_PER_SEC); }

void RawBufferVsVector() {
    auto start = Clock::now();
    vector<char> buffer(size_t(1) << 32);
    buffer[buffer.size() - 1] = '\0';
    auto end = Clock::now();
    cout << NsToSec(end - start) << endl;
    start = Clock::now();
    char* buf = new char[(size_t(1) << 32)];
    buf[buffer.size() - 1] = '\0';
    end = Clock::now();
    cout << NsToSec(end - start) << endl;
}

template <typename T>
class pod_allocator {
   public:
    // The following will be the same for virtually all allocators.
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;
    typedef std::size_t size_type;
    typedef ptrdiff_t difference_type;

    T* address(T& r) const { return &r; }

    const T* address(const T& s) const { return &s; }

    std::size_t max_size() const {
        // The following has been carefully written to be independent of
        // the definition of size_t and to avoid signed/unsigned warnings.
        return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) /
               sizeof(T);
    }

    // The following must be the same for all allocators.
    template <typename U>
    struct rebind {
        typedef pod_allocator<U> other;
    };

    bool operator!=(const pod_allocator& other) const {
        return !(*this == other);
    }

    //  template<typename U>
    //   void construct(U* p)
    //   { cout << "Called" << endl; } //it calls this *for each char element*

    void destroy(T* const p) const {}

    // Returns true if and only if storage allocated from *this
    // can be deallocated from other, and vice versa.
    // Always returns true for stateless allocators.
    bool operator==(const pod_allocator& other) const { return true; }

    // Default constructor, copy constructor, rebinding constructor, and
    // destructor. Empty for stateless allocators.
    pod_allocator() {}

    pod_allocator(const pod_allocator&) {}

    template <typename U>
    pod_allocator(const pod_allocator<U>&) {}

    ~pod_allocator() {}

    // The following will be different for each allocator.
    T* allocate(const std::size_t n) const { return new T[n]; }

    void deallocate(T* const p, const std::size_t n) const { delete[] p; }

    // The following will be the same for all allocators that ignore hints.
    template <typename U>
    T* allocate(const std::size_t n, const U* /* const hint */) const {
        return allocate(n);
    }

   private:
    pod_allocator& operator=(const pod_allocator&);
};

void PODAllocator() {
    auto start = Clock::now();
    vector<char, pod_allocator<char>> buffer(size_t(1) << 32);
    buffer[buffer.size() - 1] = '\0';
    auto end = Clock::now();
    cout << NsToSec(end - start) << endl;
    start = Clock::now();
    char* buf = new char[(size_t(1) << 32)];
    buf[buffer.size() - 1] = '\0';
    end = Clock::now();
    cout << NsToSec(end - start) << endl;
}

class RawBuffer {
   public:
    RawBuffer(size_t size, size_t alignment = sizeof(void*))
        : data_(nullptr), size_(0), alignment_(alignment), pageLocked_(false) {
        // only way to report failure in constructor is to throw
        // exceptions, check for size after construction, if zero
        // an error occurred
        Allocate(size, alignment);
    }
    RawBuffer(const RawBuffer& other) {
        Allocate(other.size_, other.alignment_);
#ifndef NO_STD_COPY
        if (size_) {
            // will call the right function e.g. __memcpy_avx_unaligned() etc.
            // but required including <algorithm>
            std::copy(other.data_, other.data_ + size_, data_);
        }
#else
        memcpy(data_, other.data_, size_);
#endif
    }
    RawBuffer(RawBuffer&& other) {
        size_ = other.size_;
        data_ = other.data_;
        alignment_ = other.alignment_;
        pageLocked_ = other.pageLocked_;
        other.data_ = nullptr;
    }
    ~RawBuffer() { Destroy(); }
    const char* Data() const { return data_; }
    char* Data() { return data_; }
    size_t Size() const { return size_; };
    size_t Alignment() const { return alignment_; }
    char& operator[](size_t i) { return data_[i]; }
    char operator[](size_t i) const { return data_[i]; }  // no ref required

   private:
    void Allocate(size_t size, size_t alignment) {
        data_ = static_cast<char*>(aligned_alloc(alignment, size));
        if (data_) size_ = size;
    }
    void Destroy() {
        if (!data_) return;
        if (pageLocked_) munlock(data_, size_);
        free(data_);
    }

   private:
    char* data_;
    size_t size_;
    size_t alignment_;
    bool pageLocked_;

   private:
    // only used by friend functions to return empty buffer in case of errors
    RawBuffer() : size_(0), data_(nullptr), alignment_(0) {}
    friend RawBuffer PageLockedBuffer(size_t);
    friend void CopyBuffer(const RawBuffer& src, RawBuffer& dest);
    // friend RawBuffer MMAlignedBuffer(size_t, size_t); //_mm_malloc/free of
    // intrinsics TBD
};

RawBuffer PageLockedBuffer(size_t size) {
    RawBuffer rb(size, sysconf(_SC_PAGESIZE));
    if (!rb.Size()) return RawBuffer();
    if (mlock(rb.Data(), size)) return RawBuffer();
    return rb;
}

void CopyBuffer(const RawBuffer& src, RawBuffer& dest) {
    const size_t sz = src.size_ <= dest.size_ ? src.size_ : dest.size_;
#ifndef NO_STD_COPY
    std::copy(src.data_, src.data_ + sz, dest.data_);
#else
    memcpy(dest.data_, src.data_, sz);
#endif
}

char* begin(RawBuffer& rb) { return rb.Data(); }
char* end(RawBuffer& rb) { return rb.Data() + rb.Size(); }
const char* begin(const RawBuffer& rb) { return rb.Data(); }
const char* end(const RawBuffer& rb) { return rb.Data() + rb.Size(); }
const char* cbegin(const RawBuffer& rb) { return rb.Data(); }
const char* cend(const RawBuffer& rb) { return rb.Data() + rb.Size(); }

void FastBuffer() {
    auto start = Clock::now();
    // memory alignment changes performance, try with '1';
    RawBuffer buffer(size_t(1) << 32, 4096);
    buffer[buffer.Size() - 1] = '\0';
    auto end = Clock::now();
    cout << NsToSec(end - start) << endl;
    start = Clock::now();
    char* buf = new char[(size_t(1) << 32)];
    buf[buffer.Size() - 1] = '\0';
    end = Clock::now();
    cout << NsToSec(end - start) << endl;
}

int main(int argc, char const* argv[]) {
    FastBuffer();
    RawBuffer rb = PageLockedBuffer(1 << 30);
    return 0;
}
