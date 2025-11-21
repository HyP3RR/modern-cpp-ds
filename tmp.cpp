#include <iostream>
#include <string>
#include <vector>

template <typename T> struct scott;

struct obj {
    int arr[10];
};

struct base {

    virtual void f() { std::cout << "Base\n"; }
};

struct derived : public base {
    void f() override { std::cout << "Derived\n"; }
};

struct other : public base {
    void f() override { std::cout << "other\n"; }
};

int main() {

    int x = 5;
    int &b = x;
    obj t;
    {
        obj x;
        for (int i = 0; i < 10; i++)
            x.arr[i] = i;
        t = std::move(x);
        for (int i = 0; i < 10; i++) {
            std::cout << x.arr[i] << " ";
        }
        std::cout << "\n";
    }
    derived obj;
    base &a = static_cast<base &>(obj);
    a.f();
    obj.f();
    other &d = static_cast<other &>(
        a); // underlying obj pointer is same, so no diff to function.
    d.f();  // vptr takes the fall!
    // t dangles!! since contents of x were on internal stack frame before
    // stealing from it, the stack frame is always deallocated.
    other tt;
    base p;
    std::cout << std::addressof(tt) << "\n";
    std::cout << std::addressof(p) << "\n";
}
