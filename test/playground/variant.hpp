#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <cstdint>

namespace test
{
// Example Enums
enum class EEnumInt8  : int8_t  { INT8_1 = -1, INT8_2 = 2, INT8_3 = 3};
enum class EEnumInt16 : int16_t { INT16_1 = -100, INT16_2 = 200, INT16_3 = 3 };

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

static const AnyValueVector dealer1 {
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

static const AnyValueVector dealer_array[] = {
  {0, "Mixed Dealer", {PoschCar{450}, ToyotaCar{"Supra"}, HondaCar{true}, true, 876543210, -5, -5, 10, INT8_1, INT16_1}},
  {1, "Mixed Dealer1", {PoschCar{1450}, ToyotaCar{"Supra1"}, HondaCar{false}, false, -876543210, -15, -15, 110, INT8_2, INT16_2}},
  {2, "Mixed Dealer2", {PoschCar{2450}, ToyotaCar{"Supra2"}, HondaCar{false}, false, 2876543210, 25, 25, 110, INT8_3, INT16_3}},
};

static const int dealer_array_len = sizeof(dealer_array) / sizeof(dealer_array[0]);

}



