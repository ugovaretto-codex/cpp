//
// Generate template index list with custom generator
// Author: Ugo Varetto
//

#include <cassert>
#include <iostream>

//------------------------------------------------------------------------------
template <int S, template <int... I> class GenT, int... N>
struct Sequence : Sequence<S - 1, GenT, N..., GenT<N...>::value> {};

template <int... N>
struct Idx {};

template <template <int... I> class GenT, int... N>
struct Sequence<0, GenT, N...> {
    using Index = Idx<N...>;
};

template <int Size, template <int... I> class GenT, int... Start>
struct GenerateIndexSequence {
    static_assert(Size > 0, "Size <= 0");
    using Type = typename Sequence<Size - 1, GenT, Start...>::Index;
};

//------------------------------------------------------------------------------
template <int H, int... I>
struct Last : Last<I...> {};

template <int I>
struct Last<I> {
    enum : int { value = I };
};

template <int Pos, int H, int... T>
struct Nth {
    enum : int { value = Nth<Pos - 1, T...>::value };
};

template <int H, int... T>
struct Nth<0, H, T...> {
    enum : int { value = H };
};

//------------------------------------------------------------------------------
template <int... I>
struct Inc {  // increment last index
    enum : int { value = Last<I...>::value + 1 };
};

template <int D>
struct IncStep {
    template <int...I>
    struct Type {
        enum : int { value = Last<I...>::value + D};
    };
};

template <int... I>
struct Fibonacci {  // generate next number in Fibonacci sequence
    static_assert(sizeof...(I) > 1,
                  "Fibonacci requires size of starting sequence > 1");
    enum : int {
        value = Nth<sizeof...(I) - 2, I...>::value +
                Nth<sizeof...(I) - 1, I...>::value
    };
};


template <int Size, int Start = 0>
using IndexSequence =
    typename GenerateIndexSequence<Size, Inc, Start>::Type;

template <int Size>
using FibonacciSequence =
    typename GenerateIndexSequence<Size, Fibonacci, 1, 1>::Type;

#if __cplusplus >= 201703L
template <int... I>
void PrintIndices(Idx<I...>) {
    (..., (std::cout << I << " "));
    std::cout << std::endl;
}
#else

template <typename T>
void PrintIndicesHelper(T) {}

template <int H, int... Tail>
void PrintIndicesHelper(Idx<H, Tail...>) {
    std::cout << H << " ";
    PrintIndicesHelper(Idx<Tail...>());
}

template <int... I>
void PrintIndices(Idx<I...>) {
    PrintIndicesHelper(Idx<I...>());
    std::cout << std::endl;
}
#endif

int main(int argc, char const *argv[]) {
    std::cout << std::endl;
    const auto FS = FibonacciSequence<10>();
    PrintIndices(FS);
    using IndexStepSequence =
        typename GenerateIndexSequence<10, IncStep<3>::Type, 1>::Type;
    const auto ISS = IndexStepSequence();
    PrintIndices(ISS);
    return 0;
}
