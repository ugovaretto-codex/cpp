#include <cmath>
#include <iostream>
#include <string>
#include <tuple>

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

template <typename FT>
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

template <typename F, IndexType... I>
void PrintXIndices(const F& f, const CustomIndex<I...>&) {
    // using fold expression
    (..., (cout << f(I) << ", "));
    cout << endl;
}

#define i2f(es, m) (es << 23 | m)

constexpr int NumBits(const uint32_t i) {
    uint32_t mask = 1 << 31;
    int count = 32;
    while (!(mask & i)) {
        --count;
        mask >>= 1;
    }
    return count;
}
template <int B>
struct Bi {
    int n = B;
    enum { bits = NumBits(B) };
};

template <typename T>
constexpr void PrintBits(const T n, int start = 0,
                         const int end = 8 * sizeof(T) - 1) {
    static_assert(is_integral<T>::value);
    for (int i = end; i >= start; --i) {
        const int v = (n & (1 << i)) >> i;
        cout << v;
    }
}

template <typename T>
constexpr int Zeros(const T n) {
    static_assert(is_floating_point<T>::value);
    int z = 0;
    T N = n - int64_t(n);
    N = N < 0 ? -N : N;
    while (int64_t(N) == int64_t(10 * N)) {
        ++z;
        N = 10 * N;
    }
    return z;
}

template <typename T>
constexpr int Zerobits(const T n) {
    static_assert(is_literal_type<T>::value);
    // int cnt = 0;
    const int end = 8 * sizeof(T) - 1;
    int zeros = 0;
    for (int cnt = end; cnt >= 0; cnt--) {
        if (((n & (1 << cnt)) >> cnt) == 0)
            ++zeros;
        else
            break;
    }
    return zeros;  // cnt > 0 ? cnt - 1: 0;
}

template <typename T>
struct FloatTraits {};

template <>
struct FloatTraits<float> {
    enum : int32_t {SIGN_BIT=31, MANTISSA_LENGTH=23, EXP_LENGTH=8};
    //after right shift
    enum : uint32_t {BIT_MASK=0x80000000, 
                     MANTISSA_MASK=0x007FFFFF, 
                     EXP_MASK=0x000000FF};
    enum : int32_t {EXP_OFFSET=127};
    enum : int32_t {MIN_EXP=126};
    enum : uint32_t {POSITIVE_INFINITE=0x7F800000, 
                     NEGATIVE_INFINITE=0xFF800000};
    using IntType = int32_t;
    using UIntType = uint32_t;
};

template <>
struct FloatTraits<double> {
    enum : int64_t {SIGN_BIT=63, MANTISSA_LENGTH=52, EXP_LENGTH=11};
    //after right shift
    enum : uint64_t {BIT_MASK=0x8000000000000000,
                     MANTISSA_MASK=0x000FFFFFFFFFFFFF, 
                     EXP_MASK=0x00000000000003FF};
    enum : int64_t  {EXP_OFFSET=1023};
    enum : int64_t  {MIN_EXP=1022};
    enum : uint64_t {POSITIVE_INFINITE=0x7FF0000000000000, 
                     NEGATIVE_INFINITE=0xFFF0000000000000};
    using IntType = int64_t;
    using UIntType = uint64_t;
};



//------------------------------------------------------------------------------
// Convert 32 bit floating point number to 32 bit unsigned int. Can be used
// in template parameter list or any other scope requiring a constexpr.
// bits:
// |31  |30........23|22.....0|
//   ^        ^          ^
// |sign|exponent-127|mantissa|
// F = -1^sign x 2^exponent x 1. mantissa : normalized, x 1.****
// denormalized: exponent = 0, F = -1^sign x 2^-126 * 0.mantissa
// denormalization not supported, only floating point numbers with integer
// part in signed 32 bit integer range supported
// Algorithm, given floating point number F:
// 1) extract sign, integer part, mantissa
// 2) compute exponent E:
//  2.1) if integer part >= 1 keep on incrementing the exponent E
//       until 2^E < Integer(F), then decrement E to find the biggest exponent
//       for which 2^E < F
//  2.2) if integer part == 0 keep on incrementing the exponent E
//       until 1/2^E > Mantissa(F)
// 3) compute mantissa, F' = |F|:
//  3.0) initialize number N to 2^E or 2^(-E) if number < 1,
//       mantissa and mask to 0
//  3.1) for each bit from position 22 to 0, while N != F:
//         if N * (1 + mantissa + 1/2^bit_i) <= F'
//           set bit i in mask to 1
//           mantissa = mantissa + 1/2^bit_i
//           N = N * (1 + mantissa + 1/2^bit_i)
// 4) if |F| < 1 exponent is negative: E = -E
// 5) return bitwise or of (sign << 31, E + 127 << 23, mantissa)
// As per IEEE754 specification 127 is subtracted from exponent value, so
// it needs to be added
constexpr uint32_t IntFloat(const float f) {
    const uint32_t S = (1 << 31) & int32_t(f);
    uint32_t I = S ? -int32_t(f) : int32_t(f);
    float M = S ? -f - I : f - I;
    int E = 0;
    if (I > 0) {
        while ((1 << E) < I) ++E;
        E = E > 0 ? E - 1 : 0;
    } else {
        E = 1;
        while (1.f / (1 << E) > M) ++E;
    }
    const float F = I + M;
    float N = I > 0 ? 1 << E : (1.f / (1 << E));
    float m = 0.f;
    uint32_t mantissa = 0;
    for (int bit = 1; bit != 24; ++bit) {
        const float r = 1.f / (1 << bit);
        const float n = N * (1.f + m + r);
        if (F - n >= 0) {
            mantissa |= 1 << (23 - bit);
            m += r;
        }
        if (F == n) break;
    }
    E = I > 0 ? E : -E;
    const uint32_t X = ((E + 127) & 0x0000000FF) << 23;
    return S | X | mantissa;
}

//------------------------------------------------------------------------------
// Convert 32 bit unsigned integer to 32 bit floating point number.
// Can be used in template parameter list or any other scope requiring
// a constexpr.
// Extract sign and exponent and sum bits in
// mantissa (sum of 1/2^(22-bit position + 1)) where bit position in [0,22]
constexpr float FloatInt(const uint32_t i) {
    const float sign = (1 << 31) & i ? -1.f : 1.f;
    int e = (((i & 0x7FFFFFFF) >> 23) & 0x000000FF) - 127;
    const float factor = e > 0 ? 1 << e : 1.f / (1 << -e);
    float mantissa = 0.f;
    for (int bit = 0; bit != 23; ++bit) {
        const int offset = 22 - bit;
        const int b = (i & (1 << offset)) >> offset;
        if (b) mantissa += 1.f / (1 << (bit + 1));
    }
    return sign * factor * (1.f + mantissa);
}


//------------------------------------------------------------------------------
constexpr uint32_t mask(int bits, int offset = 0) {
    return (((1 << bits) ^ (1 << bits)) | 1 << bits) << offset;
}

template <uint32_t F>
struct TF {
    enum : uint32_t { value = F };
};

constexpr uint32_t operator"" _f(long double f) { return IntFloat(float(f)); }

template <IndexType... ArgsT>
struct Div2 {
    enum E1 : IndexType {
        second = E1(NthElement<sizeof...(ArgsT) - 2, ArgsT...>::value)
    };
    enum E2 : IndexType {
        first = E2(NthElement<sizeof...(ArgsT) - 1, ArgsT...>::value)
    };
    enum V : IndexType {
        value = IntFloat(FloatInt(V(first)) / FloatInt(V(second)))
    };
};

//------------------------------------------------------------------------------
int main(int argc, char const* argv[]) {
#if 0
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
    // PrintIndices(MakeCustomIndexSequence<10, Div, F2I(120.45f),
    // F2I(30.567f)>::Type()); //in order for this to work they have to approve:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1330r0.pdf
    // first
    const int fi = F2I(1.0f);
    cout << hex << fi << endl;

    const uint32_t N = 124 << 23 | 1 << 21;
    cout << I2F(i2f(uint32_t(0x7F), uint32_t(3) << 19)) << endl;
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
#endif
    union U {
        uint32_t i;
        float f;
    } u;
    u.f = 11.5625f;
    cout << u.i << endl;
    PrintBits(u.i);
    cout << endl;
    u.i = IntFloat(u.f);
    cout << u.i << endl;
    PrintBits(u.i);
    cout << endl;
    TF<IntFloat(10.456f)> tf;
    cout << tf.value << endl;
    cout << Zeros(1.00045f) << endl;
    TF<10.456_f> tf2;
    cout << tf2.value << endl;
    u.f = 999999999.000001;
    PrintBits(u.i);
    cout << endl;
    PrintBits(IntFloat(u.f));
    cout << endl;
    u.f = 0.0000001;
    PrintBits(u.i);
    cout << endl;
    PrintBits(IntFloat(u.f));
    cout << endl;
    cout << Zerobits(0x7F) << endl;
    u.f = 10.1234;
    cout << FloatInt(u.i) << endl;
    using It = IndexType;
    PrintXIndices([](IndexType i) {return FloatInt(i);},
        MakeCustomIndexSequence<10, Div2, IntFloat(1.3f),
                                         IntFloat(10.0f)>::Type());
    return 0;
}

// 0 01111111 10010000000000000000000
// 1 10000001 10010000000000000000000