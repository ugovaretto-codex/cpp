//Author Ugo Varetto ugovaretto@gmail.com
//Minimal Zip iterator.
//Example of how to use variadic templates, fold expressions and 
//custom for(v: collection) loops.

#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

template <typename... ArgsT>
class Zipper {
  private:
    using Indices =
        std::make_index_sequence<tuple_size<tuple<ArgsT...>>::value>;
    using ValueTuple = tuple<typename ArgsT::value_type&...>;
    tuple<ArgsT...> its_;

   public:
    Zipper() = delete;
    Zipper(ArgsT... i) : its_(i...) {}
    Zipper(const Zipper&) = default;
    Zipper(Zipper&&) = default;
    Zipper& operator++() {
        IncIterators(Indices{});
        return *this;
    }
    ValueTuple operator*() const { return Values(Indices{}); }
    bool operator==(const Zipper& other) const {
        return Equal(other, Indices{});
    }
    bool operator!=(const Zipper& other) const { return !operator==(other); }

   private:
    template <size_t... I>
    void IncIterators(const index_sequence<I...>&) {
        (get<I>(its_)++, ...);
    }
    template <size_t... I>
    ValueTuple Values(const index_sequence<I...>&) const {
        return ValueTuple(*get<I>(its_)...);
    }
    template <size_t... I>
    bool Equal(const Zipper& other, const index_sequence<I...>&) const {
        return (... && (get<I>(its_) == get<I>(other.its_)));
    }
};

template <typename... ArgsT>
pair<Zipper<typename ArgsT::iterator...>,
     Zipper<typename ArgsT::iterator...>> constexpr Zip(ArgsT&... seqs) {
    return {Zipper<typename ArgsT::iterator...>(begin(seqs)...),
            Zipper<typename ArgsT::iterator...>(end(seqs)...)};
}

template <typename... ArgsT>
pair<Zipper<typename ArgsT::const_iterator...>,
     Zipper<typename ArgsT::
                const_iterator...>> constexpr Zip(const ArgsT&... seqs) {
    return {Zipper<typename ArgsT::iterator...>(begin(seqs)...),
            Zipper<typename ArgsT::iterator...>(end(seqs)...)};
}

template <typename F, typename S>
F constexpr begin(pair<F, S> p) {return p.first;}

template <typename F, typename S>
F constexpr end(pair<F, S> p) {return p.second;}

int main(int argc, char const* argv[]) {
    vector<int> ints{1, 2, 3};
    vector<string> strings{"1", "2", "3"};
    
    Zipper zzip(begin(ints), begin(strings));
    Zipper zzend(end(ints), end(strings));
    do {
        auto [i, s] = *zzip;
        cout << "{" << i << ", " << s << "} ";
    } while (++zzip != zzend);

    auto z = Zip(ints, strings);

    do {
        auto [i, s] = *z.first;
        cout << "{" << i << ", " << s << "} ";
    } while (++z.first != z.second);
    cout << endl;

    auto it = Zip(ints, strings);

    for(auto [i, x]: it) {
        cout << i << " " << x << endl;
    }

    return 0;
}
