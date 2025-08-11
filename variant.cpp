#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

// Example Enums
enum class EEnumInt8  : int8_t  { INT8_1 = -1, INT8_2 = 2 };
enum class EEnumInt16 : int16_t { INT16_1 = -100, INT16_2 = 200 };

// Example car types
struct PoschCar {
    int horsepower;
};
struct ToyotaCar {
    std::string model;
};
struct HondaCar {
    bool hybrid;
};

// AnyValueVector definition
struct AnyValueVector {
    int index;
    std::string name;
    std::vector<std::variant<
        PoschCar, ToyotaCar, HondaCar,
        bool, int64_t, int8_t,
        EEnumInt8, EEnumInt16
    >> car_list;
};

using enum EEnumInt8;
using enum EEnumInt16;

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
}

int main() {
    AnyValueVector dealer1 {
        0,
        "Mixed Dealer",
        {
            PoschCar{450},
            ToyotaCar{"Supra"},
            HondaCar{true},
            true,
            876543210,
            -5,
            -5,
            10,
            INT8_1,
            INT16_1
        }
    };

    DumpAnyValueVector(dealer1);
    return 0;
}

