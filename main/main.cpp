#include <Arduino.h>
#include <sdkconfig.h>
#include <esp_log.h>

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
#include "decrypt.hpp"
#include "sensor/sensor_types.hpp"
#include "sensor/sensor_region.hpp"
#include "sensor/sensor_state.hpp"
#include "sensor/sensor_serial.hpp"
#include "nfc.hpp"
#include "sensor/sensor.hpp"
#include "crc.hpp"
#include "fram.hpp"

#include <esp_task_wdt.h>

#include <stdio.h>
#include <memory>
#include <format>
#include <iostream>
#include <iomanip>

// #include <logger.hpp>

#define WDT_TIMEOUT 60 //5 seconds WDT
#define TAG "CGM_SCANNER"
// const char* TAG = "CGM_SCANNER";
// logger::set_tag("CGM_SCANNER");

// ESP-32 <-> PN5180 pinout mapping
#define PN5180_NSS  12  // GPIO12
#define PN5180_BUSY 13  // GPIO13
#define PN5180_RST  14  // GPIO14

// ESP-32 <-> SSD1306 pinout mapping
#define OLED_MOSI 5
#define OLED_CLK  4
#define OLED_DC   1
#define OLED_CS   2
#define OLED_RESET 3

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// const int relayPin = 25;  // GPIO25
// int relayState = LOW;   // relayState used to set the relayPin

// Global NFC 
PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

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

const uint8_t arrow_down_diagonal[] = {0x08, 0x1c, 0x2a, 0x49, 0x08, 0x08, 0x08, 0x08};

void drawGlucoseDisplay(double glucose) {
  display.clearDisplay();
  display.cp437(true);

  // Format glucose text
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%.1f", glucose);

  // Set glucose value on the left side
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor((SCREEN_WIDTH * 0.1), (SCREEN_HEIGHT / 2) - 9); // vertically centered
  display.println(buffer);

  // Set arrow icon on the right side
  display.drawBitmap((SCREEN_WIDTH * 0.75), (SCREEN_HEIGHT / 2), arrow_down_diagonal, 8, 8, SSD1306_WHITE);

  display.display();
}

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

    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "UploadedL %s %s", __DATE__, __TIME__);

    esp_task_wdt_config_t wdtConfig = {
        .timeout_ms = WDT_TIMEOUT * 1000,
        .idle_core_mask = 0,
        .trigger_panic = true
    };

    esp_task_wdt_init(&wdtConfig); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

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
    drawGlucoseDisplay(5.8);
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
        std::vector<uint8_t> patch_info = {0xC5, 0x09, 0x30, 0x01, 0x00, 0x00};
        // std::vector<uint8_t> patch_info = {0xC6, 0X09, 0X31, 0X01, 0X16, 0X0A};
        sensor.m_patch_info.assign(patch_info.begin(), patch_info.end());

        /// Check sensor is Libre 2 EU (currently only sensor tested) ///
        if (cgm::get_sensor_type(sensor.m_patch_info) != cgm::sensor_type::LIBRE2EU) {
            ESP_LOGE(TAG, "Sensor is not Libre 2 EU");
            continue;
        }

        /// Read sensor FRAM and decrypt ///
        std::vector<uint8_t> FRAM_vector;
        FRAM_vector.reserve(43 * blockSize);

        cgm::read_FRAM(nfc, sensor.m_uid, blockSize, FRAM_vector);

        sensor.m_fram_data = cgm::FRAM_data(cgm::decrypt_FRAM(sensor.m_uid, sensor.m_patch_info, FRAM_vector));
        if (sensor.m_fram_data.error) {
            ESP_LOGE(TAG, "Error decrypting FRAM, returning to main loop");
            // TODO: display error on screen?
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
        ESP_LOGI(TAG, "Error: %d Negative %d Raw glucose: %d Raw temperature: %d Temperature adjustment: %d",
            sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1].has_error,
            sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1].negative,
            sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1].raw_glucose,
            sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1].raw_temperature,
            sensor.m_fram_data.trend_records[sensor.m_fram_data.trend_index - 1].temperature_adjustment);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second
        } else { // If a card is not detected

            if (rc == ISO15693_EC_OK) { // If UID present, but not TI
                ESP_LOGI(TAG, "Card detected is not Texas Instruments");
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
