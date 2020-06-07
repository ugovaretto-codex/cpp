#include <iostream>
#include <string>
#include <tuple>
#include <cmath>

using namespace std;

//------------------------------------------------------------------------------
// Compile-time float <--> int bitwise conversion
template <typename FT>
struct ToInt {};

template <>
struct ToInt<float> {
    using Type = uint32_t;
};

template <>
struct ToInt<double> {
    using Type = uint64_t;
};

template <typename IT>
struct ToFloat {};

template <>
struct ToFloat<uint32_t> {
    using Type = float;
};

template <>
struct ToFloat<uint64_t> {
    using Type = double;
};


template <typename FT >
union FloatIntConvert {
    using Float = FT;
    using IntT = typename ToInt<Float>::Type;
    const IntT int_;
    const Float float_;
    constexpr FloatIntConvert(const Float f) : float_(f) {}
    constexpr FloatIntConvert(const IntT i) : int_(i) {}
    constexpr operator IntT() const { return int_; }
    constexpr operator Float() const { return float_; }
};

//------------------------------------------------------------------------------
template <typename FloatT>
constexpr typename ToInt<FloatT>::Type F2I(const FloatT f) {
    return FloatIntConvert<FloatT>(f);
}

template <typename IntT>
constexpr typename ToFloat<IntT>::Type I2F(const IntT i) {
    return FloatIntConvert<typename ToFloat<IntT>::Type>(i).float_;
}

//------------------------------------------------------------------------------
namespace {
template <typename F>
void IterateImpl(F) {}

template <typename F, typename HeadT, typename... TailT>
void IterateImpl(F f, const HeadT& head, const TailT&... tail) {
    f(head);
    IterateImpl(f, tail...);
}
}  // namespace
template <typename F, typename... ArgsT>
void Iterate(F f, const ArgsT&... args) {
    IterateImpl(f, args...);
}

namespace {
template <typename... ArgsT>
void Expand(const ArgsT&...) {}
}  // namespace

template <typename F, typename... ArgsT>
void Iterate2(F f, const ArgsT&... args) {
    // by returning a value the function can be invoked as part of a call to
    // to another function, triggering the parameter expansion
    auto e = [f](auto i) {
        f(i);
        return 0;
    };
    tuple<ArgsT...> r(e(args)...);
    // OR
    Expand(e(args)...);
}

//------------------------------------------------------------------------------
namespace {
template <typename T, typename F>
T ApplyImpl(const T& init, F) {
    return init;
}

template <typename T, typename F, typename HeadT, typename... TailT>
T ApplyImpl(const T& init, F f, const HeadT& head, const TailT&... tail) {
    return T(f(head, ApplyImpl(init, f, tail...)));
}
}  // namespace

template <typename T, typename F, typename... ArgsT>
T Apply(const T& init, F f, const ArgsT&... args) {
    return ApplyImpl(init, f, args...);
}

template <int S, int... N>
struct Sequence : Sequence<S - 1, N..., sizeof...(N)> {};

template <int... N>
struct Idx {
    const size_t size = sizeof...(N);
    const int index[sizeof...(N)] = {N...};
    int operator[](size_t i) const { return index[i]; }
};

template <int... N>
struct Sequence<0, N...> {
    using Index = Idx<N...>;
};

template <int Size, int Start = 0>
struct MakeIndexSequence {
    static_assert(Size > 0 && Start < Size);
    using Type = typename Sequence<Size - 1, Start>::Index;
};

//-----------------------------------------------------------------------------
using IndexType = int;
template <IndexType H, IndexType... Tail>
struct Head {
    enum : IndexType { value = H };
};

template <int N, IndexType H, IndexType... ArgsT>
struct NthElement : NthElement<N - 1, ArgsT...> {};

template <IndexType H, IndexType... ArgsT>
struct NthElement<0, H, ArgsT...> {
    enum : IndexType { value = H };
};

template <IndexType... ArgsT>
struct SizeOf {
    enum : IndexType { value = sizeof...(ArgsT) };
};

template <IndexType... ArgsT>
struct Fibonacci {
    enum E1 : IndexType {
        second = E1(NthElement<sizeof...(ArgsT) - 2, ArgsT...>::value)
    };
    enum E2 : IndexType {
        first = E2(NthElement<sizeof...(ArgsT) - 1, ArgsT...>::value)
    };
    enum V : IndexType { value = V(first) + V(second) };
};

template <IndexType... ArgsT>
struct Div {
    enum E1 : IndexType {
        second = E1(NthElement<sizeof...(ArgsT) - 2, ArgsT...>::value)
    };
    enum E2 : IndexType {
        first = E2(NthElement<sizeof...(ArgsT) - 1, ArgsT...>::value)
    };
    enum V : IndexType { value = F2I(I2F(V(first)) / I2F(V(second))) };
};

template <int S, template <IndexType... ArgsT> class GenT, IndexType... N>
struct CustomSequence : CustomSequence<S - 1, GenT, N..., GenT<N...>::value> {};

template <IndexType... I>
struct CustomIndex {};

template <template <IndexType... ArgsT> class GenT, IndexType... N>
struct CustomSequence<0, GenT, N...> {
    using Index = CustomIndex<N...>;
};

template <int Size, template <IndexType... ArgsT> class GenT,
          IndexType... Start>
struct MakeCustomIndexSequence {
    static_assert(Size > 0);
    using Type = typename CustomSequence<Size - 1, GenT, Start...>::Index;
};

//------------------------------------------------------------------------------
template <int... I>
void PrintIndices(const Idx<I...>& ii) {
    auto print = [=](auto i) {
        cout << i << endl;
        return 0;
    };
    Expand(print(I)...);
}

template <IndexType... I>
void PrintIndices(const CustomIndex<I...>&) {
    // using fold expression
    (..., (cout << I << ", "));
    cout << endl;
}

#define i2f(es, m) (es << 23 | m)

constexpr int NumBits(const uint32_t i) {
    uint32_t mask = 1 << 31;
    int count = 32;
    while(!(mask & i)) {
        --count;
        mask >>= 1;
    }
    return count;
}
template <int B>
struct Bi {
    int n = B;
    enum {bits=NumBits(B)};
};

constexpr uint32_t IntMantissa(const float f) {
    const float m = f - float(uint32_t(f));
    float fi = m * 10;
    while(fi - uint32_t(fi) > 0.f) fi *= 10;
    return uint32_t(fi);
}

int IntExponent(const float f) {
    const float m = 1.0f + (f - uint32_t(f));
    cout << f << " " << m << endl;
    int e = -126;
    while(((1 << e) * m < f) && e <= 127) {
        cout << ( 1 << e) * m <<  " " << e << endl;
        ++e;
    }
    return e;
}

uint32_t IntFloat(const float f) {
    //1) compute exponent

    //2) loop: biggest exponent E : 2 ^ E <= f
    const uint32_t sign = 1 << 31 & uint32_t(f);
    uint32_t e = 0;
    while((1 << e) < uint32_t(f)) ++e;
    --e;
    //3) mantissa = 0

    //4) loop bit 0 -> 22: 2 ^ E x (1 + mantissa) -> F
                // if f - F >= 0
                //   mask |= 1 << (22-bit)
                //   mantissa += 1/2^bit
    float m = 0.f;
    uint32_t mask = 0;
    float r = (2 << e) * (1.0f + m);
    for(int bit = 0; bit != 22; ++bit) {
        if(r == f) break;
        const float r2 = 2 << e * (1.0f + m);
        if((f-r2) > 0 && (f-r2) < (f-r)) {
            mask |= 1 << bit;
            mantissa += (1.f / (1 << (bit + 1)));
            r = r2;
        }
    }
    //5) return sign | E | mask 
    return sign | (e - 127) | mask;
}



constexpr uint32_t mask(int bits, int offset = 0) {
    return (((1 << bits) ^ (1 << bits)) | 1 << bits) << offset;
}

// constexpr uint32_t IntFloat(const float f) {
//     const uint32_t integral = uint32_t(f);
//     const uint32_t mantissa = IntMantissa(f);
// }
// constexpr uint32_t FloatInt(const uint32_t sexp, const int mantissa) {
//     (sexp - 0x7F) << 23 | mantiss << ()
// }

//------------------------------------------------------------------------------
int main(int argc, char const* argv[]) {
    Iterate([](const int& i) { cout << i << endl; }, 1, 2, 3, 4, 5, 6, 7);

    Iterate2([](const int& i) { cout << i << endl; }, 1, 2, 3, 4, 5, 6, 7);

    auto sum = [](auto i1, auto i2) { return i1 + i2; };

    auto mul = [](auto i1, auto i2) { return i1 * i2; };

    const int identity_int = 0;
    const int res1 = Apply(identity_int, sum, 1, 2, 3, 4, 5, 6, 7);
    cout << res1 << endl;

    const int identity_mul = 1;
    const int res2 = Apply(1, mul, 1, 2, 3, 4, 5, 6, 7);
    cout << res2 << endl;

    auto I = Sequence<3, 0>::Index();
    cout << I.index[2] << endl;

    PrintIndices(MakeIndexSequence<4>::Type());

    cout << "=========================" << endl;
    using It = IndexType;
    PrintIndices(MakeCustomIndexSequence<10, Fibonacci, It(1), It(1)>::Type());
    //PrintIndices(MakeCustomIndexSequence<10, Div, F2I(120.45f),
    //F2I(30.567f)>::Type()); //in order for this to work they have to approve:
    //http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1330r0.pdf
    //first
    const int fi = F2I(1.0f);
    cout << hex << fi << endl;

    const uint32_t N = 124 << 23 | 1 << 21;
    cout << I2F(i2f(uint32_t(0x7F),uint32_t(3) << 19)) << endl;
    cout << I2F(N) << endl;
    cout << NumBits(0x7f) << endl;
    Bi<4> bs;
    cout << bs.bits << endl;
    cout << dec << IntMantissa(13.1234f) << endl;
    const uint32_t im = IntMantissa(13.1234f);
    cout << ((im >> 4) << 4) << endl;
    cout << IntExponent(1.5625f) << endl;
    cout << IntMantissa(1.5625f) << endl;
    cout << NumBits(5625) << endl;
    uint32_t f = 127 << 23 | 5625 << 9;
    union U {
        uint32_t i;
        float f;
    } u;
    u.i = f;
    cout << u.f << endl;
    u.f = 1.5625f;
    cout << hex << u.i << endl;
    return 0;
}