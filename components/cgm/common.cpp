/*
 * Code adapted from:
 * https://github.com/gui-dos/DiaBLE
 * https://github.com/creepymonster/GlucoseDirect
 * https://github.com/j-kaltes/Juggluco
 * https://github.com/NightscoutFoundation/xDrip
*/

#include <common.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

namespace cgm {

int read_bits(const std::vector<uint8_t>& buffer, int byte_offset, int bit_offset, int bit_count) {
    if (bit_count == 0) {
        return 0;
    }

    int res = 0;
    for (auto i = 0; i < bit_count; i++) {
        int total_bit_offset = byte_offset * 8 + bit_offset + i;
        int byte = std::floor(total_bit_offset / 8);
        int bit = total_bit_offset % 8;
        if (byte >= 0 && byte < buffer.size() && ((buffer[byte] >> bit) * 0x1) == 1) {
            res |= (1 << i);
        }
    }

    return res;
}

std::vector<uint16_t> prepare_variables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
    uint16_t s1 = static_cast<uint16_t>(static_cast<uint32_t>((uid[5] << 8) | uid[4]) + static_cast<uint32_t>(x) + static_cast<uint32_t>(y));
    uint16_t s2 = static_cast<uint16_t>(static_cast<uint32_t>((uid[3] << 8) | uid[2]) + static_cast<uint32_t>(keys[2]));
    uint16_t s3 = static_cast<uint16_t>(static_cast<uint32_t>((uid[1] << 8) | uid[0]) + static_cast<uint32_t>(x) * 2);
    uint16_t s4 = 0x241a ^ keys[3];

    return {s1, s2, s3, s4};
}

std::vector<uint16_t> process_crypto(const std::vector<uint16_t>& input) {
    auto op = [](uint16_t value) -> uint16_t {
        uint16_t res = value >> 2; // result does not include these last 2 bits

        if ((value & 1) != 0) { // if last bit is 1
            res = res ^ keys[1];
        }

        if ((value & 2) != 0) { // if second last bit is 1
            res = res ^ keys[0];
        }

        return res;
    };

    uint16_t r0 = op(input[0]) ^ input[3];
    uint16_t r1 = op(r0) ^ input[2];
    uint16_t r2 = op(r1) ^ input[1];
    uint16_t r3 = op(r2) ^ input[0];
    uint16_t r4 = op(r3);
    uint16_t r5 = op(r4 ^ r0);
    uint16_t r6 = op(r5 ^ r1);
    uint16_t r7 = op(r6 ^ r2);

    uint16_t f1 = r0 ^ r4;
    uint16_t f2 = r1 ^ r5;
    uint16_t f3 = r2 ^ r6;
    uint16_t f4 = r3 ^ r7;

    return {f4, f3, f2, f1};
}

std::vector<uint8_t> useful_function(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
    auto block_key = process_crypto(prepare_variables(uid, x, y));
    auto low = block_key[0];
    auto high = block_key[1];

    // uint8_t r1 = low ^ 0x4163;
    // uint8_t r2 = high ^ 0x4344;
    uint16_t r1 = low ^ 0x4163;
    uint16_t r2 = high ^ 0x4344;

    uint8_t r1_low = r1 & 0xFF;
    uint8_t r1_high = (r1 >> 8) & 0xFF;
    uint8_t r2_low = r2 & 0xFF;
    uint8_t r2_high = (r2 >> 8) & 0xFF;

    // uint8_t f1 = r1 >> 8;
    // uint8_t f2 = r2 >> 8;

    // return {r1, f1, r2, f2};
    return {r1_low, r1_high, r2_low, r2_high};
}

} // namespace cgm
