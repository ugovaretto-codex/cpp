//
// Generate template index list with offset, C++ version >= 201103 
// Author: Ugo Varetto
//

#include <iostream>
#include <cassert>

template <int H, int...I>
struct Last : Last<I...>{};

template<int I>
struct Last<I> {
    enum : int {value = I};
};


template <int... N>
struct Idx {
    const size_t size = sizeof...(N);
    const int index[sizeof...(N)] = {N...};
    int operator[](size_t i) const { return index[i]; }
};

template <int S, int... N>
struct Sequence : Sequence<S - 1, N..., int(Last<N...>::value) + 1> {};

template <int... N>
struct Sequence<0, N...> {
    using Index = Idx<N...>;
};

template <int Size, int Start = 0>
struct MakeIndexSequence {
    static_assert(Size > 0, "Size <= 0");
    static_assert(Start < Size, "Start >= Size");
    using Type = typename Sequence<Size - 1, Start>::Index;
};

#if __cplusplus >= 201703L
template <int...I>
void PrintIndices(Idx<I...>) {
    (..., (std::cout << I << " "));
    std::cout << std::endl;
}
#else
template <typename T>
void PrintIndicesHelper(T) {}

template <int H, int...Tail>
void PrintIndicesHelper(Idx<H, Tail...>) {
    std::cout << H << " ";
    PrintIndicesHelper(Idx<Tail...>());
}

template <int...I>
void PrintIndices(Idx<I...>) {
    PrintIndicesHelper(Idx<I...>());
    std::cout << std::endl;
}
#endif

int main(int argc, char const *argv[]) {
    const auto IS = MakeIndexSequence<10, 3>::Type();
    for(int i = 3; i != 10; ++i) {
        assert(IS[i-3] == i);
    }
    constexpr int Size = 3;
    constexpr int Start = 1;
    PrintIndices(MakeIndexSequence<Size, Start>::Type());
    return 0;
}
