#include <nfc.hpp>

#include <crc.hpp>

#include "PN5180ISO15693.h"

namespace cgm {

ISO15693ErrorCode CGM_NFC::read_FRAM(uint8_t* uid, const uint8_t block_size, std::vector<uint8_t>& buffer) {
    auto rc = ISO15693_EC_OK;
    auto num_blocks = 43;

    for (auto num = 0; num < num_blocks; num++) {
        uint8_t block_data[block_size];

        auto rc = readSingleBlock(uid, num, block_data, block_size);

        if (rc != ISO15693_EC_OK) {
            Serial.print("Error reading block ");
            Serial.println(num);
            return rc;
        } else {
            for (auto i = 0; i < block_size; i++) {
                buffer.push_back(block_data[i]);
            }
        }
    }

    return rc;
}

} // namespace cgm
