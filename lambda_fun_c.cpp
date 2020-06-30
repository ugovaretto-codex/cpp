#include <iostream>

using Fun = int (*)(int);

template <typename RetT, typename...ArgsT>
struct Callable {
    using Fun = Ret (*)(void*, ArgsT...);
    Fun fun = nullptr;
    void* id = nullptr;
    void (destroy*)(void*) = nullptr;
    RetT operator()(ArgsT...args) {
        return fun(id, args...);
    }
    ~Callable() { destroy(id); }
};

    

Fun MakeFun(int offset) {
    thread_local int I = 0;
    I = offset;
    struct __ {
        static int F(int i) {
            thread_local int J = I;
            return i + J;
        }
    };
    return &__::F;
}

int main(int argc, char const *argv[]) {
    auto f = MakeFun(3);
    auto f2 = MakeFun(5);
    std::cout << f(4) << " " << f2(4) << std::endl;
    return 0;
}
