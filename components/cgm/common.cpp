#include <common.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

namespace cgm {

int readBits(const std::vector<uint8_t>& buffer, int byte_offset, int bit_offset, int bit_count) {
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

} // namespace cgm
