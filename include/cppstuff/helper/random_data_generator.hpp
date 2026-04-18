/*
Copyright (c) 2025, 2026 Gary Huang (deepkh@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <random>

#include <array>
#include <cstdint>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream>
#include <string>
#include <utility>

namespace random_data_generator {

// Generate random data and fill into buffer
// Returns number of bytes written, or -1 if error
static int gen(uint8_t *buf, int size) {
    if (!buf || size <= 0) {
        return -1;
    }

    // Use C++11 random engine
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister
    std::uniform_int_distribution<uint16_t> dist(0, 255);

    for (int i = 0; i < size; ++i) {
        buf[i] = static_cast<uint8_t>(dist(gen));
    }

    return size;
}

// Compute MD5 checksum and return as hex string
static std::pair<std::string, std::array<char, MD5_DIGEST_LENGTH>> md5(uint8_t *buf, int size) {
    std::pair<std::string, std::array<char, MD5_DIGEST_LENGTH>> ret;
    ret.first = "";

    if (!buf || size <= 0) {
        return ret;
    }

    unsigned char result[MD5_DIGEST_LENGTH];
    MD5(buf, size, result);

    std::ostringstream oss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(result[i]);
    }

    ret.first = oss.str();
    memcpy(ret.second.data(), result, MD5_DIGEST_LENGTH);
    return ret;
}

} // namespace random_data_generator
