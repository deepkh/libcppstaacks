#include "variant.hpp"

using namespace test;

// Dump function
void DumpAnyValueVector(const AnyValueVector& av) {
    std::cout << "Index: " << av.index << ", Name: " << av.name << "\n";
    for (const auto& item : av.car_list) {
        std::visit([](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, PoschCar>) {
                std::cout << "PoschCar { horsepower: " << v.horsepower << " }\n";
            }
            else if constexpr (std::is_same_v<T, ToyotaCar>) {
                std::cout << "ToyotaCar { model: " << v.model << " }\n";
            }
            else if constexpr (std::is_same_v<T, HondaCar>) {
                std::cout << "HondaCar { hybrid: " << (v.hybrid ? "true" : "false") << " }\n";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                std::cout << "bool: " << (v ? "true" : "false") << "\n";
            }
            else if constexpr (std::is_same_v<T, int64_t>) {
                std::cout << "int64_t: " << v << "\n";
            }
            else if constexpr (std::is_same_v<T, int8_t>) {
                std::cout << "int8_t: " << static_cast<int>(v) << "\n";
            }
            else if constexpr (std::is_same_v<T, EEnumInt8>) {
                std::cout << "EEnumInt8: " << static_cast<int>(v) << "\n";
            }
            else if constexpr (std::is_same_v<T, EEnumInt16>) {
                std::cout << "EEnumInt16: " << static_cast<int>(v) << "\n";
            }
        }, item);
    }
            std::cout << std::endl;
}

int main() {
    DumpAnyValueVector(dealer1);

    int len = dealer_array_len;
    std::cout << "len: " << len << std::endl;

    for (int i=0; i<len; i++) {
      DumpAnyValueVector(dealer_array[i]);
    }
    
    return 0;
}

