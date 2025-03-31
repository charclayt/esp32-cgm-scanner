#include <crc.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

bool check_CRC16(const std::vector<uint8_t>& data, uint8_t type) {
    auto num_bytes_to_read = 0;

    if (type == 0) { // header
        num_bytes_to_read = 24;
    } else if (type == 1) { // body
        num_bytes_to_read = 296;
    } else if (type == 2) { // footer
        num_bytes_to_read = 24;
    }

    // convert and store the first two bytes of the data (crc16) as a uint16_t
    uint16_t crc_truth = data.at(0) | (data.at(1) << 8);

    // compute the CRC16 of the data ...

    uint16_t crc = 0xFFFF;

    // Calculate CRC for each byte in the data (except the first two bytes, which is the CRC16)
    for (auto i = 2; i < num_bytes_to_read; ++i) {
        crc = (uint16_t)((crc >> 8) ^ CRC16_table[(crc ^ data.at(i)) & 0xFF]);
    }

    // CGM uses a reversed CRC16
    uint16_t reverse_crc = 0;
    for (auto i = 0; i < 16; i++) {
        reverse_crc = (uint16_t)((uint16_t)(reverse_crc << 1) | (uint16_t)(crc & 1));
        crc >>= 1;
    }

    // return whether the computed CRC16 matches the stored CRC16
    return reverse_crc == crc_truth;
}

} // namespace cgm
