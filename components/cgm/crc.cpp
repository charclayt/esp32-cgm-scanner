#include <crc.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

bool check_CRC16(const std::vector<uint8_t>& data, size_t crcByte1, size_t crcByte2) {
    // convert and store the bytes at the specified positions as the CRC16
    uint16_t crc_truth = data[crcByte1] | (data[crcByte2] << 8);

    // compute the CRC16 of the data ...

    uint16_t crc = 0xFFFF;

    // Calculate CRC for each byte in the data (except the CRC bytes)
    for (size_t i = 0; i < data.size(); i++) {
        if (i != crcByte1 && i != crcByte2) {
            crc = (uint16_t)((crc >> 8) ^ CRC16_table[(crc ^ data[i]) & 0xFF]);
        }
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

// bool check_CRC16(const std::vector<uint8_t>& data) {
//     // convert and store the first two bytes of the data (crc16) as a uint16_t
//     uint16_t crc_truth = data[0] | (data[1] << 8);

//     // compute the CRC16 of the data ...

//     uint16_t crc = 0xFFFF;

//     // Calculate CRC for each byte in the data (except the first two bytes, which is the CRC16)
//     for (auto i = 2; i < data.size(); i++) {
//         crc = (uint16_t)((crc >> 8) ^ CRC16_table[(crc ^ data[i]) & 0xFF]);
//     }

//     // CGM uses a reversed CRC16
//     uint16_t reverse_crc = 0;
//     for (auto i = 0; i < 16; i++) {
//         reverse_crc = (uint16_t)((uint16_t)(reverse_crc << 1) | (uint16_t)(crc & 1));
//         crc >>= 1;
//     }

//     // return whether the computed CRC16 matches the stored CRC16
//     return reverse_crc == crc_truth;
// }

} // namespace cgm
