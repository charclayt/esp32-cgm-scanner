#include <Arduino.h>
#include <sdkconfig.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <common_main.hpp>
#include <display_utils.hpp>
// #include <ble_main.hpp>

// PN5180 includes (NFC reader)
#include "PN5180.h"
#include "PN5180ISO15693.h"

// Display includes (SSD1306)
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// BLE includes
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"


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

#define WDT_TIMEOUT 20 // 20 seconds WDT
#define TAG "CGM_SCANNER"

// Global NFC 
cgm::CGM_NFC nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

// Global display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI,
                         OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// BLE variables
bool device_found = false;

// Global sensor
// cgm::sensor sensor;

// Global VSPI object for SPI communication - used for OLED display (PN5180 uses HSPI and manages internally)
SPIClass vspi(2);

extern "C" void app_main() {
    vspi.begin(OLED_CLK, OLED_DC, OLED_MOSI, OLED_CS);

    esp_log_level_set("*", ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "UploadedL %s %s", __DATE__, __TIME__);

    ESP_LOGI(TAG, "Free heap size: %" PRIu32, esp_get_free_heap_size());

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

    // BLE
    // nimble_port_init();
    // ble_svc_gap_init();
    // ble_svc_gatt_init();

    // ble_hs_cfg.sync_cb = []() {
    //     ESP_LOGI(TAG_BLE, "BLE host synced");
    //     // Do not start scan immediately — wait until NFC reads a valid sensor
    // };

    // nimble_port_freertos_init([](void *param) {
    //     ESP_LOGI(TAG, "NimBLE host task started");
    //     nimble_port_run();
    // });

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
        std::vector<uint8_t> patch_info;
        patch_info.reserve(6);
        auto ret = nfc.get_sensor_info(patch_info);
        if (ret != ISO15693_EC_OK) {
            ESP_LOGE(TAG, "Error getting sensor info");
            display_error(display, "Error getting sensor info");
            continue;
        }
        ESP_LOGD(TAG, "get_sensor_info ret: %d", ret);

        /// Check sensor is Libre 2 EU (currently only sensor tested) ///
        // if (cgm::get_sensor_type(sensor.m_patch_info) != cgm::sensor_type::LIBRE2EU) {
        //     ESP_LOGE(TAG, "Sensor is not Libre 2 EU");
        //     display_error(display, "Incompatible sensor");
        //     continue;
        // }

        /// Read sensor FRAM and decrypt ///
        std::vector<uint8_t> FRAM_vector;
        FRAM_vector.reserve(43 * blockSize);

        rc = nfc.read_FRAM(uid, blockSize, FRAM_vector);
        if (rc != ISO15693_EC_OK) {
            ESP_LOGE(TAG, "Error reading FRAM");
            display_error(display, "Error reading FRAM");
            continue;
        }

        sensor.initialise(uid, patch_info.data(), FRAM_vector);

        if (sensor.m_fram_data.error) {
            ESP_LOGE(TAG, "Error decrypting FRAM, returning to main loop");
            display_error(display, "Error decrypting FRAM");
            continue;
        }

        /// log sensor details ///
        ESP_LOGI(TAG, "Sensor serial: %s", sensor.m_serial_numer.c_str());
        ESP_LOGI(TAG, "Sensor family: %s", cgm::to_string(sensor.m_family).c_str());
        ESP_LOGI(TAG, "Sensor region: %s", cgm::to_string(sensor.m_region).c_str());
        ESP_LOGI(TAG, "Sensor state: %s", cgm::to_string(sensor.m_state).c_str());
        ESP_LOGI(TAG, "Sensor type: %s", cgm::to_string(sensor.m_type).c_str());

        ESP_LOGI(TAG, "Trend pointer: %d", sensor.m_fram_data.trend_index);
        ESP_LOGI(TAG, "Historic pointer: %d", sensor.m_fram_data.historic_index);

        ESP_LOGI(TAG, "Current trend record %d", sensor.m_fram_data.trend_index);

        // DEBUG - log all FRAM trend records
        for (int i = 0; i < sensor.m_fram_data.trend_records.size(); i++) {
            const auto& record = sensor.m_fram_data.trend_records[i];
            ESP_LOGD(TAG, "FRAM TREND RECORD %d - Error: %d, Negative %d, Raw glucose: %f, Raw temperature: %f, Temperature adjustment: %f, Calibrated glucose: %d",
                i + 1,
                record.has_error,
                record.negative,
                record.raw_glucose,
                record.raw_temperature,
                record.temperature_adjustment,
                record.glucose_value);
        }

        std::vector<uint8_t> ble_mac_address;
        ble_mac_address.reserve(6);
        rc = nfc.initiate_ble_connection(sensor.m_uid, sensor.m_patch_info, ble_mac_address);
        if (rc == ISO15693_EC_OK) {
            // memcpy(ble_main::target_ble_address, ble_mac_address.data(), 6);
            // ble_main::ble_target_set = true;

            // ESP_LOGI(TAG, "Target BLE address set: %02X:%02X:%02X:%02X:%02X:%02X",
            //         ble_main::target_ble_address[5], ble_main::target_ble_address[4], ble_main::target_ble_address[3],
            //         ble_main::target_ble_address[2], ble_main::target_ble_address[1], ble_main::target_ble_address[0]);

            // // Start scanning for sensor
            // ble_main::start_scan();
        } 
        // else {
        //     display_error(display, "Error initiating BLE connection");
        //     continue;   
        // }

        /// Get most recent trend record without error and time it was taken ///
        auto latest_record_no_error = sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1];
        auto time_since_reading = 0;
        if (latest_record_no_error.has_error || latest_record_no_error.glucose_value == 0) {
            ESP_LOGI(TAG, "Most recent trend record has error, getting closest valid record");
            auto found_valid_record = false;
            auto contiguous_records = cgm::calculate_contiguous_records(sensor.m_fram_data.trend_records, sensor.m_fram_data.trend_index, false);
            for (auto i = 1; i < contiguous_records.size(); i++) {
                time_since_reading += 1; // add 1 minute for each record
                if (!contiguous_records[i].has_error && contiguous_records[i].glucose_value != 0) {
                    latest_record_no_error = contiguous_records[i];
                    found_valid_record = true;
                    ESP_LOGI(TAG, "Found valid record %d, which is %d minutes ago", i + 1, time_since_reading);
                    break;
                }
            }
            // for (int i = sensor.m_fram_data.trend_index - 2; i != sensor.m_fram_data.trend_index - 1; i--) {
            //     time_since_reading += 1; // add 1 minute for each record
            //     if (!sensor.m_fram_data.trend_records[i].has_error && sensor.m_fram_data.trend_records[i].glucose_value != 0) {
            //         latest_record_no_error = sensor.m_fram_data.trend_records[i];
            //         found_valid_record = true;
            //         ESP_LOGI(TAG, "Found valid record %d, which is %d minutes ago", i + 1, time_since_reading);
            //         break;
            //     }
            //     // Loop back around to the end of fram_data.trend_records, and continue until fram_data.trend_index - 1
            //     if (i == 0) {
            //         i = cgm::NUM_FRAM_TREND_RECORDS;
            //     }
            // }
            if (!found_valid_record) {
                ESP_LOGE(TAG, "No valid record found");
                display_error(display, "No valid record found");
                continue;
            }
        }

        /// Display current NFC values on screen ///
        // auto trend = cgm::calculate_glucose_roc(cgm::calculate_contiguous_records(sensor.m_fram_data.trend_records, sensor.m_fram_data.trend_index, true));

        auto current_glucose = cgm::calculate_glucose_mmol(latest_record_no_error.glucose_value);

        // auto predicted_glucose = cgm::calculate_glucose_mmol(cgm::calculate_glucose_15_minute_predicton(trend, latest_record_no_error));

        // draw_glucose_display(display, current_glucose, time_since_reading, trend, predicted_glucose);

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
