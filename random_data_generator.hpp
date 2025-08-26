#pragma once

#include <cstdint>
#include <random>
#include <algorithm>

#include <openssl/md5.h>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <array>
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
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(result[i]);
    }

    ret.first = oss.str();
    memcpy(ret.second.data(), result, MD5_DIGEST_LENGTH);
    return ret;
}




}
