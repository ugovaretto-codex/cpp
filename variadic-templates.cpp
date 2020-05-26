#include <iostream>
#include <tuple>
#include <string>

using namespace std;

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
    //by returning a value the function can be invoked as part of a call to
    //to another function, triggering the parameter expansion
    auto e = [f](auto i){f(i); return 0;};
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

template <int S, int...N>
struct Sequence : Sequence<S-1, N...,sizeof...(N)>
{};


template <int...N>
struct Idx {
    const size_t size = sizeof...(N);
    const int index[sizeof...(N)] = {N...};
    int operator[](size_t i) const {
        return index[i];
    }
};

template <int...N>
struct Sequence<0, N...> {
    using Index = Idx<N...>;
};

template <int Size, int Start = 0>
struct MakeIndexSequence {
    static_assert(Size > 0 && Start < Size);
    using Type = typename Sequence<Size-1, Start>::Index;
};

template <int...I>
void PrintIndices(const Idx<I...>&) {
    auto print = [](auto i){cout << sizeof...(I) - i - 1 << endl; return 0;};
    Expand(print(I)...);
}

//------------------------------------------------------------------------------
int main(int argc, char const* argv[]) {
    Iterate([](const int& i) { cout << i << endl; }, 1, 2, 3, 4, 5, 6, 7);

    Iterate2(
        [](const int& i) {
            cout << i << endl;
        },
        1, 2, 3, 4, 5, 6, 7);

    auto sum = [](auto i1, auto i2) { return i1 + i2; };

    auto mul = [](auto i1, auto i2) { return i1 * i2; };

    const int identity_int = 0;
    const int res1 = Apply(identity_int, sum, 1, 2, 3, 4, 5, 6, 7);
    cout << res1 << endl;

    const int identity_mul = 1;
    const int res2 = Apply(1, mul, 1, 2, 3, 4, 5, 6, 7);
    cout << res2 << endl;


    auto I = Sequence<3,0>::Index();
    cout << I.index[2] << endl;

    PrintIndices(MakeIndexSequence<4>::Type());

    return 0;
    
}
