#include <record.hpp>

#include <common.hpp>

#include <vector>
#include <cstdint>
#include <algorithm>

namespace cgm {

uint8_t reverse_bits(uint8_t input) {
    uint8_t output = input;
    for (int i = sizeof(input) * 8 - 1; i; --i) {
        output <<= 1;
        input >>= 1;
        output |= input & 1;
    }
    return output;
}

/*
(pre-reversal) FRAM record layout:

gggg gggg gggg ----
---- ---- -ett tttt
tttt ttaa aaaa aaan

g = raw glucose
e = has error
t = raw temperature
a = temperature adjustment
n = negative

(post-reversal) FRAM record layout:

naaa aaaa aatt tttt
tttt tte- ---- ----
---- gggg gggg gggg
*/

fram_record::fram_record(const std::vector<uint8_t> record) {
    // Check if FRAM record is valid (6 bytes)
    if (record.size() != FRAM_RECORD_SIZE) {
        has_error = true;
        return;
    }

    // TODO: could just pass in copy of record rather than reference
    auto input = record;

    // reverse the bytes in the record
    std::reverse(input.begin(), input.end());

    // reverse the bits in each byte
    for (auto& byte : input) {
        byte = reverse_bits(byte);
    }

    // negative bit is now the first bit (bit 0)
    negative = (input[0] >> 7) & 0x01;

    // temperature_adjustment bits are now the final 7 bits of byte 0 and the first 2 bits of byte 1 (bits 1-9)
    temperature_adjustment = (((input[0] & 0x7F) << 2) | (input[1] >> 6)) << 2;

    // raw_temperature bits are now the final 6 bits of the byte 1 and the first 6 bits of byte 2 (bits 10-21)
    raw_temperature = (((input[1] & 0x3F) << 6) | (input[2] >> 2)) << 2;

    // has_error bit is now the 7th bit of byte 2 (bit 22)
    has_error = (input[2] >> 1) & 0x01;

    // raw_glucose bits are now the final byte and the final 4 bits of byte 4 (bits 36-47)
    raw_glucose = ((input[4] & 0x0F) << 8) | input[5];
}

} // namespace cgm
