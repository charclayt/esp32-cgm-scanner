#include <nfc.hpp>

#include <crc.hpp>

#include "PN5180ISO15693.h"

namespace cgm {

void read_FRAM(PN5180ISO15693& nfc, std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer) {
    auto num_blocks = 43;

    for (auto num = 0; num < num_blocks; num++) {
        uint8_t block_data[block_size];

        auto rc = nfc.readSingleBlock(uid.data(), num, block_data, block_size);

        if (rc != ISO15693_EC_OK) {
            Serial.print("Error reading block ");
            Serial.println(num);
        } else {
            for (auto i = 0; i < block_size; i++) {
                buffer.push_back(block_data[i]);
            }
        }
    }
}

void readHeader(PN5180ISO15693& nfc, std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer) {
    auto num_blocks = 3;

    // Read the first three blocks of the FRAM (0, 1, 2)
    for (auto block = 0; block < num_blocks; ++block) {
        uint8_t block_data[block_size];

        auto rc = nfc.readSingleBlock(uid.data(), block, block_data, block_size);

        if (rc != ISO15693_EC_OK) {
            Serial.print("Error reading block ");
            Serial.println(block);
        } else {
            for (auto i = 0; i < block_size; ++i) {
                buffer.push_back(block_data[i]);
            }
        }
    }

    // Check if the buffer is full (three blocks * block_size)
    if (buffer.size() == (block_size * num_blocks)) {
        if (check_CRC16(buffer, (uint8_t) 0)) {
            Serial.println("CRC16 check for header successful");
        } else {
            Serial.println("CRC16 check for header failed");
        }
    } else {
        Serial.println("Error reading header: buffer size is too small");
    }

}

} // namespace cgm
