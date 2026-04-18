#include <iostream>
#include <memory>

struct B; // Forward declaration

struct A {
    std::shared_ptr<B> b_ptr; // Owns B
    ~A() { std::cout << "A destroyed\n"; }
};

struct B {
    std::shared_ptr<A> a_ptr; // Owns A
    ~B() { std::cout << "B destroyed\n"; }
};

int main() {
    auto a = std::make_shared<A>();
    auto b = std::make_shared<B>();

    a->b_ptr = b;
    b->a_ptr = a;

    // At the end of main, both ref counts are still > 0
    // => Memory leak: destructors never called
}
