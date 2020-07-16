#include <iostream>

using namespace std;

#ifdef BL
void Foo(int i) {
    using F = void (*)(int);
    struct __ {
        static void True(int i) { cout << i << " < 10" << endl; }
        static void False(int i) { cout << i << " >= 10" << endl; }
    };
    static const F condition[] = {__::True, __::False};
    condition[i < 0](i);
}
#else
void Foo(int i) {
    if(i<10) { cout << i << " < 10" << endl; }
    else { cout << i << " >= 10" << endl; }
}
#endif
int Gen(int g) {
    return g / 4;
}
int main(int argc, char const *argv[]) {
    
    Foo(Gen(9));
    return 0;
}