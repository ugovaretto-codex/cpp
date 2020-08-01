// C++20 tuple implementation
// author: Ugo Varetto

#include <iostream>

using namespace std;

//------------------------------------------------------------------------------
template <typename H, typename... T>
struct Tuple : Tuple<T...> {
    H val;
    constexpr Tuple() = default;
    constexpr Tuple(const H& h, const T&... t) : Tuple<T...>(t...), val(h) {}
    constexpr Tuple(const Tuple& other)
        : Tuple<T...>(static_cast<const Tuple<T...>&>(other)), val(other.val) {}
    Tuple& operator=(const Tuple& other) {
        val = other.val;
        // cast to base
        Tuple<T...>::operator=(static_cast<const Tuple<T...>&>(other));
        return *this;
    }
};

template <typename T>
struct Tuple<T> {
    T val;
    constexpr Tuple() = default;
    constexpr Tuple(const T& v) : val(v) {}
    constexpr Tuple(const Tuple& other) = default;
    Tuple& operator=(const Tuple& other) {
        val = other.val;
        return *this;
    }
};

//------------------------------------------------------------------------------
template <int Position, typename H, typename... T>
struct GetType {
    using Type = typename GetType<Position - 1, T...>::Type;
};

template <typename H, typename... T>
struct GetType<0, H, T...> {
    using Type = H;
};

//------------------------------------------------------------------------------
template <int Pos, typename H, typename... ArgsT>
const typename GetType<Pos, H, ArgsT...>::Type& Get(
    const Tuple<H, ArgsT...>& t) {
    // cast to base
    return Get<Pos - 1>(static_cast<const Tuple<ArgsT...>&>(t));
}

template <int Pos, typename H, typename... ArgsT>
const typename GetType<Pos, H, ArgsT...>::Type& Get(
    const Tuple<H, ArgsT...>& t) requires(Pos == 0) {  // end of iteration
    return t.val;
}

template <int Pos, typename H, typename... ArgsT>
typename GetType<Pos, H, ArgsT...>::Type& Get(Tuple<H, ArgsT...>& t) {
    // cast to base
    return Get<Pos - 1>(static_cast<Tuple<ArgsT...>&>(t));
}

template <int Pos, typename H, typename... ArgsT>
typename GetType<Pos, H, ArgsT...>::Type& Get(Tuple<H, ArgsT...>& t) requires(
    Pos == 0) {  // end of iteration
    return t.val;
}
//------------------------------------------------------------------------------
template <typename H, typename... ArgsT>
constexpr bool operator==(const Tuple<H, ArgsT...>& t1,
                          const Tuple<H, ArgsT...>& t2) {
    using Base = Tuple<ArgsT...>;
    if constexpr (sizeof...(ArgsT) == 1)
        return t1.val == t2.val;
    else
        return t1.val == t2.val &&  // error without else part of constexpr
               static_cast<const Base&>(t1) == static_cast<const Base&>(t2);
}

template <typename H, typename... ArgsT>
std::ostream& operator<<(std::ostream& os, const Tuple<H, ArgsT...>& t) {
    os << t.val << ' ';
    // "if" *must* be evaluated at compile time
    // if not compiler will try to generate
    // the code int the "if" body for Tuple with zero elements
    if constexpr (sizeof...(ArgsT) > 0)
        os << static_cast<const Tuple<ArgsT...>&>(t);
    return os;
}

//------------------------------------------------------------------------------
int main(int argc, char const* argv[]) {
    Tuple<int, float, char> t = {2, 2.4f, 'c'};
    cout << t << endl;
    Tuple<int, float, char> t2 = {2, 2.4f, 'c'};
    cout << boolalpha << (t == t2) << endl;
    Get<2>(t2) = 'X';
    cout << Get<2>(t2) << endl;
    return 0;
}
