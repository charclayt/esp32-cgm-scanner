#include <Arduino.h>
#include <sdkconfig.h>
#include <esp_log.h>

#include <common_main.hpp>
#include <display_utils.hpp>

// PN5180 includes (NFC reader)
#include "PN5180.h"
#include "PN5180ISO15693.h"

// Display includes (SSD1306)
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BLE includes
// #include "NimBLEDevice.h"

// CGM includes
#include "calculations.hpp"
#include "common.hpp"
#include "crc.hpp"
#include "decrypt.hpp"
#include "fram.hpp"
#include "nfc.hpp"
#include "sensor/sensor_types.hpp"
#include "sensor/sensor_region.hpp"
#include "sensor/sensor_state.hpp"
#include "sensor/sensor_serial.hpp"
#include "sensor/sensor.hpp"
#include "trend.hpp"

#include <esp_task_wdt.h>

#include <stdio.h>
#include <memory>
#include <format>
#include <iostream>
#include <iomanip>

// #include <logger.hpp>

#define WDT_TIMEOUT 60 //5 seconds WDT
#define TAG "CGM_SCANNER"

// Global NFC 
cgm::CGM_NFC nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

// Global display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI,
                         OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// BLEScan* pBLEScan;
bool device_found = false;

// Global sensor
// cgm::sensor sensor;

// Global VSPI object for SPI communication - used for OLED display (PN5180 uses HSPI and manages internally)
SPIClass vspi(2);

std::string targetSensorName = ""; // TODO: this should be sensor serial number

// TODO: move / fix
// class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
//     void onResult(BLEAdvertisedDevice advertisedDevice) {
//         if (std::string(advertisedDevice.getName().c_str()).find(targetSensorName) != std::string::npos) {
//             BLEDevice::getScan()->stop();

//             Serial.println(F("Found target sensor!"));
//             device_found = true;

//             // BLEClient*  pClient  = BLEDevice::createClient();
//             // pClient->connect(&advertisedDevice);
//             // BLERemoteService* pRemoteService = pClient->getService("FDE3");
//             // if (pRemoteService != nullptr) {
//             //     BLERemoteCharacteristic* pLoginCharacteristic = pRemoteService->getCharacteristic("F001");
//             //     if (pLoginCharacteristic != nullptr) {
//             //         // Enable notifications and write login credentials
//             //         pLoginCharacteristic->registerForNotify([](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
//             //             // Handle notifications from the sensor
//             //         });
//             //         String loginCredential = ""; // Set the login credential here
//             //         pLoginCharacteristic->writeValue(loginCredential, false);
//             //     }
//             // }
//         }
//     }
// };


extern "C" void app_main() {
    vspi.begin(OLED_CLK, OLED_DC, OLED_MOSI, OLED_CS);

    esp_log_level_set("*", ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "UploadedL %s %s", __DATE__, __TIME__);

    esp_task_wdt_config_t wdtConfig = {
        .timeout_ms = WDT_TIMEOUT * 1000,
        .idle_core_mask = 0,
        .trigger_panic = true
    };

    esp_task_wdt_init(&wdtConfig); // enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); // add current thread to WDT watch

    // initialise NFC reader
    nfc.begin();
    nfc.reset();
    nfc.setupRF();

    ESP_LOGI(TAG, "PN5180 ISO15693 working");

    // initialise display (retry 5 times)
    for (auto i = 0; i < 5; i++) {
        if(!display.begin(SSD1306_SWITCHCAPVCC)) {
            ESP_LOGE(TAG, "SSD1306 allocation failed");
            if (i == 4) {
                ESP_LOGE(TAG, "Failed to initialise display");
                return;
            }
        } else {
            break;
        }
    }
    
    display.display();

    display.clearDisplay();

    ///// TODO: default display - if sensor stored, display that -> else "no sensor found" /////
    draw_default_display(display);
    /////

    cgm::sensor sensor;
    
while (true) {
    esp_task_wdt_reset();

    /// Try to read a tag ID (or "get inventory" in ISO15693-speak) ///
    uint8_t uid[8];
    ISO15693ErrorCode rc = nfc.getInventory(uid);
    ESP_LOGD(TAG, "getInventory returned: %d", rc);

    sensor.set_UID(uid);
    // sensor.m_uid.assign(uid, uid + sizeof(uid));

    /// If the result code was that a card had been read, and is TI tag ///
    if (rc == ISO15693_EC_OK && uid[6] == cgm::TEXAS_INSTRUMENTS) {
        ESP_LOGI(TAG, "Texas Instruments tag detected");
        
        // Format each byte as HEX, padded with leading zeros if required
        std::stringstream uid_output;
        for (int i = 7; i >= 0; i--) {
            uid_output << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(uid[i]);
        }
        std::string uid_string = uid_output.str();
        ESP_LOGI(TAG, "Tag UID: %s", uid_string.c_str());

        /// Get system info for Block Size and Number of Blocks ///
        uint8_t blockSize, numBlocks;
        rc = nfc.getSystemInfo(uid, &blockSize, &numBlocks);

        ESP_LOGD(TAG, "getSystemInfo returned: %d", rc);
        ESP_LOGI(TAG, "Block size: %d, Number of blocks: %d", blockSize, numBlocks);

        /// Get patch info ///
        uint8_t buffer[7] = {};
        auto ret = nfc.getPatchInfo(buffer);
        ESP_LOGD(TAG, "getPatchInfo ret: %d", ret);
        // TODO: fix getPatchInfo, remove hardcoded values
        // std::vector<uint8_t> patch_info = {0xC5, 0x09, 0x30, 0x01, 0x00, 0x00};
        std::vector<uint8_t> patch_info = {0xC6, 0X09, 0X31, 0X01, 0X45, 0X07};
        sensor.m_patch_info.assign(patch_info.begin(), patch_info.end());

        /// Check sensor is Libre 2 EU (currently only sensor tested) ///
        if (cgm::get_sensor_type(sensor.m_patch_info) != cgm::sensor_type::LIBRE2EU) {
            ESP_LOGE(TAG, "Sensor is not Libre 2 EU");
            display_error(display, "Incompatible sensor");
            continue;
        }

        /// Read sensor FRAM and decrypt ///
        std::vector<uint8_t> FRAM_vector;
        FRAM_vector.reserve(43 * blockSize);

        nfc.read_FRAM(sensor.m_uid, blockSize, FRAM_vector);

        sensor.m_fram_data = cgm::FRAM_data(cgm::decrypt_FRAM(sensor.m_uid, sensor.m_patch_info, FRAM_vector));
        if (sensor.m_fram_data.error) {
            ESP_LOGE(TAG, "Error decrypting FRAM, returning to main loop");
            display_error(display, "Error decrypting FRAM");
            continue;
        }

        /// set and log sensor details ///
        sensor.m_state = cgm::get_sensor_state(sensor.m_fram_data.sensor_state);
        ESP_LOGI(TAG, "Sensor state: %s", cgm::to_string(sensor.m_state).c_str());

        sensor.m_region = cgm::get_sensor_region(sensor.m_patch_info);
        ESP_LOGI(TAG, "Sensor region: %s", cgm::to_string(sensor.m_region).c_str());

        sensor.m_serial_numer = cgm::get_sensor_serial_number(sensor.m_uid, sensor.m_patch_info);
        ESP_LOGI(TAG, "Sensor serial: %s", sensor.m_serial_numer.c_str());

        ESP_LOGI(TAG, "Trend pointer: %d", sensor.m_fram_data.trend_index);
        ESP_LOGI(TAG, "Historic pointer: %d", sensor.m_fram_data.historic_index);

        ESP_LOGI(TAG, "Current trend record %d", sensor.m_fram_data.trend_index);

        // DEBUG - log all FRAM trend records
        for (int i = 0; i < sensor.m_fram_data.trend_records.size(); i++) {
            const auto& record = sensor.m_fram_data.trend_records[i];
            ESP_LOGD(TAG, "FRAM TREND RECORD %d - Error: %d Negative %d Raw glucose: %d Raw temperature: %d Temperature adjustment: %d",
                i + 1,
                record.has_error,
                record.negative,
                record.raw_glucose,
                record.raw_temperature,
                record.temperature_adjustment);
        }

        /// Get most recent trend record without error and time it was taken ///
        // TODO: modify this once fram trends are restructured in order of time
        auto latest_record_no_error = sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1];
        auto time_since_reading = 0;
        if (latest_record_no_error.has_error) {
            ESP_LOGI(TAG, "Most recent trend record has error, getting closest valid record");
            auto found_valid_record = false;
            // auto contiguous_records = cgm::calculate_contiguous_records(sensor.m_fram_data.trend_records, sensor.m_fram_data.trend_index, true);
            for (int i = sensor.m_fram_data.trend_index - 2; i != sensor.m_fram_data.trend_index - 1; i--) {
                time_since_reading += 1; // add 1 minute for each record
                if (!sensor.m_fram_data.trend_records[i].has_error) {
                    latest_record_no_error = sensor.m_fram_data.trend_records[i];
                    found_valid_record = true;
                    ESP_LOGI(TAG, "Found valid record %d, which is %d minutes ago", i + 1, time_since_reading);
                    break;
                }
                // Loop back around to the end of fram_data.trend_records, and continue until fram_data.trend_index - 1
                if (i == 0) {
                    i = cgm::NUM_FRAM_TREND_RECORDS;
                }
            }
            if (!found_valid_record) {
                ESP_LOGE(TAG, "No valid record found");
                // TODO: display ERR on CGM screen
                display_error(display, "No valid record found");
                continue;
            }
        }

        /// Display current NFC values on screen ///
        // auto trend = cgm::calculate_glucose_roc(cgm::calculate_contiguous_records(sensor.m_fram_data.trend_records, sensor.m_fram_data.trend_index, true));

        auto current_glucose = cgm::calculate_glucose_mmol(latest_record_no_error);

        // draw_glucose_display(display, current_glucose, time_since_reading, trend);

        auto contiguous_historic_records = cgm::calculate_contiguous_records(sensor.m_fram_data.historic_records, sensor.m_fram_data.historic_index, false);
        draw_historic_display(display, current_glucose, contiguous_historic_records);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
        } else { // If a card is not detected

            if (rc == ISO15693_EC_OK) { // If UID present, but not TI
                ESP_LOGI(TAG, "Card detected is not Texas Instruments");
                display_error(display, "Not TI sensor");
                vTaskDelay(900 / portTICK_PERIOD_MS);
            } else {
                // The most significant (last) byte of a valid UID should always be 0xE0. e.g. E007C4A509C247A8                    
                ESP_LOGI(TAG, "No card detected");
                vTaskDelay(900 / portTICK_PERIOD_MS);
            }

        #ifdef DEBUG
        ESP_LOGE(TAG, "Error when reading: %d", rc);
        #endif
    }
}
}
