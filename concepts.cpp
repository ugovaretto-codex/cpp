//Author: Ugo Varetto

#include <iostream>
#include <concepts>
#include <iterator>
#include <list>
#include <vector>
#include <ranges>

using namespace std;


template <forward_iterator I> constexpr void Advance(I& it, ptrdiff_t inc) {
    if(inc > 0) {
        while(inc--) ++it;
    } else if (inc < 0) {
        while(inc++) --it;
    }
}

template <random_access_iterator I> constexpr void Advance(I& it, ptrdiff_t inc) {
    it += inc;
}


template <typename T>
concept IsClass = is_class<T>::value;

template <typename T>
struct S {
    void P(T) requires IsClass<T> {
    }
};


template <int A>
concept Fizz = A % 3 == 0;

template <int A>
concept Buzz = A % 5 == 0;

template <int A>
void FB() requires Fizz<A> { cout << ' ' << "Fizz";}

template <int A>
void FB() requires Buzz<A> { cout << ' ' << "Buzz";}

template <int A>
void FB() requires Fizz<A> && Buzz<A> { cout << ' ' << "FizzBuzz";}

template <int A>
void FB() { cout << ' '<< A;}

template<int...I>
void FizzBuzz() {
    (...,FB<I>());
    cout << endl;
}

template <const char* C>
struct Str {};

constexpr char CIAO[] = "ciao";
constexpr char HELLO[] = "hello";

template <typename S>
concept Hello = is_same<S, Str<HELLO>>::value;

template <typename S>
void P(S) requires Hello<S>
{}

template <Hello H>
void Q(H) {}

template <typename F1, typename F2>
concept divisible = requires(F1 f1, F2 f2) { f1 / f2 ;};

static_assert(divisible<int, float>);

template <typename T>
concept number = is_integral<T>::value || is_floating_point<T>::value;

//template <typename...ArgsT>
//concept integers = (... && is_integral<ArgsT>::value)


template <typename HeadT, typename...TailT>
struct Head {
    using Type = HeadT;
};


template <typename...NumbersT>
constexpr typename Head<NumbersT...>::Type
Sum(NumbersT...nums) requires (... && number<NumbersT>) {
    using T = typename Head<NumbersT...>::Type;
    return (... + T(nums));
}

int main(int argc, char const *argv[]) {
    cout << Sum(1.0, 2, 4.f) << endl;
    S<int> s;
    //s.P(); //compilation error
    FizzBuzz<1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>();
    for(int i = 2; i > 0; i--) cout << i << endl;
    vector<int> vi = {1,2,3,4};
    list<int> li = {1,2,3,4};
    auto vb = begin(vi);
    auto lb = begin(li);
    Advance(vb, 2);
    Advance(lb, 2);
    cout << *vb << ' ' << *lb << endl;
    //IsClass auto c = 3;
    // concepts.cpp:67:5: error: deduced type 'int' does not satisfy 'IsClass'
    // IsClass auto c = 3;
    // ^~~~~~~
    // concepts.cpp:21:19: note: because 'is_class<int>::value' evaluated to false
    // concept IsClass = is_class<T>::value;
    return 0;
}
