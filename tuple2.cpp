#include <iostream>

using namespace std;

template <typename T, typename...RestT>
struct Tuple : Tuple<RestT...> {
    using Base = Tuple<RestT...>;
    T value;
    constexpr Tuple(const T& v, const RestT&...r) : Base(r...), value(v) {}
};

template <typename T>
struct Tuple<T> {
    T value;
    constexpr Tuple(const T& v) : value(v) {}
};

template<size_t I, typename T, typename...RestT>
constexpr const auto&  get(const Tuple<T, RestT...>& t)
requires (I == 0) {
    return t.value;
}

template<size_t I, typename T, typename...RestT>
constexpr const auto&  get(const Tuple<T, RestT...>& t)
requires (I > 0 && I < sizeof...(RestT) + 1) {
    using Base = typename Tuple<T, RestT...>::Base;
    return get<I-1>((const Base&) t);
}

template<size_t I, typename T, typename...RestT>
constexpr auto&  get(Tuple<T, RestT...>& t)
requires (I == 0) {
    return t.value;
}

template<size_t I, typename T, typename...RestT>
constexpr auto&  get(Tuple<T, RestT...>& t) 
requires (I > 0 && I < sizeof...(RestT) + 1) {
    using Base = typename Tuple<T, RestT...>::Base;
    return get<I-1>((Base&) t);
}

template <int I>
struct Int {
    static const int i = I;
};

void foo(int a, float b) {}


int main(int, char**) {
    constexpr Tuple<int, float, char> t = {1, 3.2f, 'c'};
    cout << t.value << endl;
    const auto& a = ((typename Tuple<int, float, char>::Base) t);
    cout << a.value << endl;
    constexpr auto i = get<1>(t);
    cout << i << endl;
    Int<get<2>(t)> ii;
    cout << char(ii.i) << endl;
    Tuple<char, int> ci = {'a', 3};
    get<1>(ci) = 5;
    cout << get<1>(ci) << endl;
    return 0;
}