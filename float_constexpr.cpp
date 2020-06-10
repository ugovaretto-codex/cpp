//
// constexpr float <--> int bitwise conversion, compatible with non-type
// template parameters: MyType<IntFloat(1.345f)> mt;
// Author: Ugo Varetto
//

#include <cassert>
#include <cinttypes>
#include <iostream>

using namespace std;

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
// it needs to be added back
constexpr uint32_t IntFloat(const float f) {
    const uint32_t S = (1 << 31) & int32_t(f);
    uint32_t I = S ? -int32_t(f) : int32_t(f);
    float M = S ? -f - I: f - I;
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
    uint32_t mask = 0;
    for (int bit = 1; bit != 24; ++bit) {
        const float r = 1.f / (1 << bit);
        const float n = N * (1.f + m + r);
        if (F - n >= 0) {
            mask |= 1 << (23 - bit);
            m += r;
        }
        if (F == n) break;
    }
    E = I > 0 ? E : -E;
    const uint32_t x = ((E + 127) & 0x0000000FF) << 23;
    return S | x | mask;
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
    float m = 0.f;
    for (int bit = 0; bit != 23; ++bit) {
        const int offset = 22 - bit;
        const int b = (i & (1 << offset)) >> offset;
        if (b) m += 1.f / (1 << (bit + 1));
    }
    return sign * factor * (1.f + m);
}

//------------------------------------------------------------------------------
template <uint32_t F>
struct Float {
    enum : uint32_t { value = F };
    constexpr operator float() const { return FloatInt(value); }
};

//------------------------------------------------------------------------------
template <typename T>
void PrintBits(const T n, int start = 0, const int end = 8 * sizeof(T) - 1) {
    static_assert(is_integral<T>::value);
    for (int i = end; i >= start; --i) {
        const int v = (n & (1 << i)) >> i;
        cout << v;
    }
}

//------------------------------------------------------------------------------
//WARNING: user defined literals for numeric types require the argument to
//always be positive
constexpr uint32_t operator"" _f(long double v) { return IntFloat(float(v)); }

//------------------------------------------------------------------------------
int main(int argc, char const *argv[]) {
    union U {
        uint32_t i;
        float f;
    } u;

    U fi;
    fi.f = -10.234f;
    assert(fi.i == IntFloat(fi.f));
    cout << "float number: " << fi.f << endl;
    cout << "float bits:   ";
    PrintBits(fi.i);
    cout << endl;
    cout << "uint32_t:     " << fi.i << endl;

    Float<10.234_f> f;
    assert(float(f) == 10.234f);

    return 0;
}