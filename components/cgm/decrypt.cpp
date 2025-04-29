// TODO: references

#include <decrypt.hpp>

#include <common.hpp>
#include <crc.hpp>

#include <vector>
#include <cstdint>

#include <esp_log.h>

#define TAG "CGM_DECRYPT"

namespace cgm {

std::vector<uint8_t> decrypt_FRAM(std::vector<uint8_t>& uid, const std::vector<uint8_t>& info, const std::vector<uint8_t>& encrypted_data) {
    std::vector<uint8_t> result;
    result.reserve((sizeof(uint8_t) * 8) * FRAM_SIZE_BLOCKS);

    for (auto i = 0; i < 43; ++i) {
        auto input = prepare_variables(uid, static_cast<uint16_t>(i), static_cast<uint16_t>(((info[5] << 8) | info[4]) ^ 0x44));
        auto block_key = process_crypto(input);

        result.push_back(encrypted_data[i * 8 + 0] ^ static_cast<uint8_t>(block_key[0] & 0xFF));
        result.push_back(encrypted_data[i * 8 + 1] ^ static_cast<uint8_t>((block_key[0] >> 8) & 0xFF));
        result.push_back(encrypted_data[i * 8 + 2] ^ static_cast<uint8_t>(block_key[1] & 0xFF));
        result.push_back(encrypted_data[i * 8 + 3] ^ static_cast<uint8_t>((block_key[1] >> 8) & 0xFF));
        result.push_back(encrypted_data[i * 8 + 4] ^ static_cast<uint8_t>(block_key[2] & 0xFF));
        result.push_back(encrypted_data[i * 8 + 5] ^ static_cast<uint8_t>((block_key[2] >> 8) & 0xFF));
        result.push_back(encrypted_data[i * 8 + 6] ^ static_cast<uint8_t>(block_key[3] & 0xFF));
        result.push_back(encrypted_data[i * 8 + 7] ^ static_cast<uint8_t>((block_key[3] >> 8) & 0xFF));
    }

    auto crc_header = check_CRC16(std::vector<uint8_t>(result.begin(), result.begin() + 24), 0, 1);
    ESP_LOGD(TAG, "FRAM header CRC match: %d", crc_header != 0);

    auto crc_body = check_CRC16(std::vector<uint8_t>(result.begin() + 24, result.begin() + 24 + 296), 0, 1);
    ESP_LOGD(TAG, "FRAM body CRC match: %d", crc_body != 0);

    auto crc_footer = check_CRC16(std::vector<uint8_t>(result.begin() + 24 + 296, result.begin() + 24 + 296 + 24), 0, 1);
    ESP_LOGD(TAG, "FRAM footer CRC match: %d", crc_footer != 0);

    if (crc_header == 0 || crc_body == 0 || crc_footer == 0) {
        ESP_LOGE(TAG, "FRAM CRC check failed");
        return {};
    }

    return result;
}

std::vector<uint8_t> decrypt_BLE(std::vector<uint8_t>& uid, const std::vector<uint8_t>& encrypted_data) {
    auto d = useful_function(uid, 0x1b, 0x1b6a);

    uint16_t one = static_cast<uint16_t>(d[1]) << 8 | d[0];
    uint16_t two = static_cast<uint16_t>(d[3]) << 8 | d[2];

    uint16_t x = (one ^ two) | 0x63;
    uint16_t y = static_cast<uint16_t>(encrypted_data[0]) << 8 | encrypted_data[1];

    std::vector<uint8_t> key;
    auto initial_key = process_crypto(prepare_variables(uid, x, y));

    for (auto i = 0; i < 8; i++) {
        key.push_back(static_cast<uint8_t>(initial_key[0]));
        key.push_back(static_cast<uint8_t>(initial_key[0]) >> 8);
        key.push_back(static_cast<uint8_t>(initial_key[1]));
        key.push_back(static_cast<uint8_t>(initial_key[1]) >> 8);
        key.push_back(static_cast<uint8_t>(initial_key[2]));
        key.push_back(static_cast<uint8_t>(initial_key[2]) >> 8);
        key.push_back(static_cast<uint8_t>(initial_key[3]));
        key.push_back(static_cast<uint8_t>(initial_key[3]) >> 8);
        initial_key = process_crypto(initial_key);
    }

    std::vector<uint8_t> result(encrypted_data.begin(), encrypted_data.end());
    for (auto i = 0; i < result.size(); i++) {
        result[i] ^= key[i];
    }

    // check the CRC in the final 2 bytes
    auto crc = check_CRC16(result, result[result.size() - 1], result[result.size() - 2]);
    if (crc == 0) {
        ESP_LOGE(TAG, "BLE CRC check failed");
        return {};
    }

    return result;
}

// std::vector<uint16_t> prepare_variables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
//     uint16_t s1 = static_cast<uint16_t>(static_cast<uint32_t>((uid[5] << 8) | uid[4]) + static_cast<uint32_t>(x) + static_cast<uint32_t>(y));
//     uint16_t s2 = static_cast<uint16_t>(static_cast<uint32_t>((uid[3] << 8) | uid[2]) + static_cast<uint32_t>(keys[2]));
//     uint16_t s3 = static_cast<uint16_t>(static_cast<uint32_t>((uid[1] << 8) | uid[0]) + static_cast<uint32_t>(x) * 2);
//     uint16_t s4 = 0x241a ^ keys[3];

//     return {s1, s2, s3, s4};
// }

// std::vector<uint16_t> process_crypto(const std::vector<uint16_t>& input) {
//     auto op = [](uint16_t value) -> uint16_t {
//         uint16_t res = value >> 2; // result does not include these last 2 bits

//         if ((value & 1) != 0) { // if last bit is 1
//             res = res ^ keys[1];
//         }

//         if ((value & 2) != 0) { // if second last bit is 1
//             res = res ^ keys[0];
//         }

//         return res;
//     };

//     uint16_t r0 = op(input[0]) ^ input[3];
//     uint16_t r1 = op(r0) ^ input[2];
//     uint16_t r2 = op(r1) ^ input[1];
//     uint16_t r3 = op(r2) ^ input[0];
//     uint16_t r4 = op(r3);
//     uint16_t r5 = op(r4 ^ r0);
//     uint16_t r6 = op(r5 ^ r1);
//     uint16_t r7 = op(r6 ^ r2);

//     uint16_t f1 = r0 ^ r4;
//     uint16_t f2 = r1 ^ r5;
//     uint16_t f3 = r2 ^ r6;
//     uint16_t f4 = r3 ^ r7;

//     return {f4, f3, f2, f1};
// }

// std::vector<uint8_t> useful_function(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
//     auto block_key = process_crypto(prepare_variables(uid, x, y));
//     auto low = block_key[0];
//     auto high = block_key[1];

//     uint8_t r1 = low ^ 0x4163;
//     uint8_t r2 = high ^ 0x4344;

//     uint8_t f1 = r1 >> 8;
//     uint8_t f2 = r2 >> 8;

//     return {r1, f1, r2, f2};
// }

} // namespace cgm
