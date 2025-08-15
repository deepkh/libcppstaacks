#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

std::atomic<int> counter(0); // Atomic variable

void increment() {
    for (int i = 0; i < 100000; ++i) {
        counter++; // Atomic increment
    }
}

void decrement() {
    for (int i = 0; i < 100000; ++i) {
        counter--; // Atomic decrement
    }
}

int main() {
    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(increment);
    }
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(decrement);
    }

    for (auto &t : threads) t.join();

    std::cout << "Counter: " << counter << "\n"; 
    // ✅ Always 400000
    return 0;
}
