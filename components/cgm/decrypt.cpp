/*
 * Code adapted from:
 * https://github.com/gui-dos/DiaBLE
 * https://github.com/creepymonster/GlucoseDirect
 * https://github.com/j-kaltes/Juggluco
 * https://github.com/NightscoutFoundation/xDrip
*/

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

} // namespace cgm
