#include <nfc.hpp>

#include <common.hpp>
#include <crc.hpp>

#include "PN5180ISO15693.h"

#include <esp_log.h>
#define TAG "CGM_NFC"

#include <cstring>

namespace cgm {

ISO15693ErrorCode CGM_NFC::read_FRAM(uint8_t* uid, const uint8_t block_size, std::vector<uint8_t>& buffer) {
    auto rc = ISO15693_EC_OK;

    for (auto num = 0; num < FRAM_SIZE_BLOCKS; num++) {
        uint8_t block_data[block_size];

        rc = readSingleBlock(uid, num, block_data, block_size);

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

ISO15693ErrorCode CGM_NFC::get_sensor_info(std::vector<uint8_t>& buffer) {
    auto rc = ISO15693_EC_OK;
    
    uint8_t command[] = {0x02, 0xA1, 0x07};
    uint8_t *readBuffer;

    rc = issueISO15693Command(command, sizeof(command), &readBuffer);

    ESP_LOGD(TAG, "readBuffer: %02X %02X %02X %02X %02X %02X %02X", readBuffer[0], readBuffer[1], readBuffer[2], readBuffer[3], readBuffer[4], readBuffer[5], readBuffer[6]);

    buffer.insert(buffer.end(), readBuffer + 1, readBuffer + 7);

    return rc;
}

ISO15693ErrorCode CGM_NFC::initiate_ble_connection(std::vector<uint8_t>& uid, std::vector<uint8_t>& sensor_info, std::vector<uint8_t>& buffer) {
    auto rc = ISO15693_EC_OK;

    uint16_t y;
    uint16_t code = 0x1E;

    ESP_LOGE(TAG, "sensor_info: %02X %02X %02X %02X %02X %02X", sensor_info[0], sensor_info[1], sensor_info[2], sensor_info[3], sensor_info[4], sensor_info[5]);
    ESP_LOGE(TAG, "uid: %02X %02X %02X %02X %02X %02X %02X %02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

    uint8_t b[4] = {
            static_cast<uint8_t>(m_unlock_code & 0xFF),
            static_cast<uint8_t>((m_unlock_code >> 8) & 0xFF),
            static_cast<uint8_t>((m_unlock_code >> 16) & 0xFF),
            static_cast<uint8_t>((m_unlock_code >> 24) & 0xFF)
        };
    y = (static_cast<uint16_t>(sensor_info[4]) << 8 | sensor_info[5]) ^ (static_cast<uint16_t>(b[1]) << 8 | b[0]);

    auto d = useful_function(uid, code, y);
    // // uint16_t secret = static_cast<uint16_t>((sensor_info[4] << 8) | sensor_info[5]) ^ static_cast<uint16_t>((parameters[1] << 8) | parameters[0]);

    uint8_t command[4 + 4 + 4]; // 4 subcmd, 4 unlock code, 2 secret
    command[0] = 0x02;

    command[1] = 0xA1;
    command[2] = 0x1E;
    command[3] = 0x07;

    command[4] = b[0];
    command[5] = b[1];
    command[6] = b[2];
    command[7] = b[3];
    command[8] = b[0];
    command[9] = b[1];
    command[10] = sensor_info[4];
    command[11] = sensor_info[5];
    // command[8] = d[0];
    // command[9] = d[1];
    // command[10] = d[2];
    // command[11] = d[3];

    ESP_LOGD(TAG, "BLE initialisation command: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X ", command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7], command[8], command[9], command[10], command[11]);
    // ESP_LOGD(TAG, "BLE initialisation command: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", command[0], command[1], command[2], command[3], command[4], command[5], command[6], command[7], command[8], command[9], command[10]);
    ESP_LOGD(TAG, "size of command: %d", sizeof(command));

    uint8_t* read_buffer;
    rc = issueISO15693Command(command, sizeof(command), &read_buffer);

    if (rc != ISO15693_EC_OK) {
        ESP_LOGE(TAG, "Error initiating BLE connection: %d", rc);
        return rc;
    }

    buffer.insert(buffer.end(), read_buffer, read_buffer + 6);
    return rc;
}

} // namespace cgm
