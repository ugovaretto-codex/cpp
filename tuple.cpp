// C++20 tuple implementation
// author: Ugo Varetto

// TODO: static_cast

#include <iostream>

using namespace std;

// template <int...T>
// struct Index {
//     static constexpr int i[] = {T...};
// };

// template <int S, int...T>
// struct Sequence : Sequence<S-1, T..., sizeof...(T)> {
// };

// template <int...T>
// struct Sequence<0, T...> {
//     using Type = Index<T...>;
// };

// template <int S>
// struct MakeIndex {
//     using Type = typename Sequence<S,0>::Type;
// };

//------------------------------------------------------------------------------
template <typename H, typename... T>
struct Tuple : Tuple<T...> {
    H val;
    Tuple(const H& h, const T&... t) : Tuple<T...>(t...), val(h) {}
    Tuple& operator=(const Tuple& other) {
        val = other.val;
        Tuple<T...>::operator=((const Tuple<T...>&)other);
        return *this;
    }
};

template <typename T>
struct Tuple<T> {
    T val;
    Tuple(const T& v) : val(v) {}
    Tuple& operator=(const Tuple& other) {
        val = other.val;
        return *this;
    }
};

template <int Position, typename H, typename... T>
struct GetType {
    using Type = typename GetType<Position - 1, T...>::Type;
};

template <typename H, typename... T>
struct GetType<0, H, T...> {
    using Type = H;
};

template <int Pos, typename H, typename... ArgsT>
const typename GetType<Pos, H, ArgsT...>::Type& Get(
    const Tuple<H, ArgsT...>& t) {
    using Base = Tuple<ArgsT...>;
    return Get<Pos - 1>((const Base&)t);
}

template <int Pos, typename H, typename... ArgsT>
const typename GetType<Pos, H, ArgsT...>::Type& Get(
    const Tuple<H, ArgsT...>& t) requires(Pos == 0) {
    return t.val;
}

template <typename H, typename... ArgsT>
bool operator==(const Tuple<H, ArgsT...>& t1, const Tuple<H, ArgsT...>& t2) {
    if constexpr (sizeof...(ArgsT) == 1)
        return t1.val == t2.val;
    else
        return t1.val == t2.val &&  // error without else part of constexpr
               ((const Tuple<ArgsT...>&)t1 == (const Tuple<ArgsT...>&)t2);
}

template <typename H, typename... ArgsT>
std::ostream& operator<<(std::ostream& os, const Tuple<H, ArgsT...>& t) {
    os << t.val << ' ';
    if constexpr (sizeof...(ArgsT) >
                  0)  // if *must* be evaluated at compile time
                      // if not compiler will try to generate
                      // the code for Tuple with zero elements
        os << static_cast<const Tuple<ArgsT...>&>(t);
    return os;
}

int main(int argc, char const* argv[]) {
    Tuple<int, float, char> t = {2, 2.4f, 'c'};
    cout << t << endl;
    Tuple<int, float, char> t2 = {2, 2.4f, 'c'};
    cout << boolalpha << (t == t2) << endl;
    return 0;
}
