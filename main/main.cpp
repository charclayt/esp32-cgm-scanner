#include "Arduino.h"

#include "PN5180.h"
#include "PN5180ISO15693.h"

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


#define WDT_TIMEOUT 5 //5 seconds WDT

// ESP-32 <-> PN5180 pinout mapping
#define PN5180_NSS  12  // GPIO12
#define PN5180_BUSY 13  // GPIO13
#define PN5180_RST  14  // GPIO14

const int relayPin = 25;  // GPIO25
int relayState = LOW;   // relayState used to set the relayPin

// Global NFC 
PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

// Global sensor
std::unique_ptr<cgm::sensor> sensor;


void setup() {
    pinMode(relayPin, OUTPUT);
    Serial.begin(115200);
    digitalWrite(relayPin, relayState);
    delay(500);

    esp_task_wdt_config_t wdtConfig = {
        .timeout_ms = WDT_TIMEOUT * 1000,
        .idle_core_mask = 0,
        .trigger_panic = true
    };

    esp_task_wdt_init(&wdtConfig); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    Serial.println(F("Uploaded: " __DATE__ " " __TIME__)); Serial.println(" ");

    nfc.begin();
    nfc.reset();
    nfc.setupRF();

    Serial.println(F("PN5180 ISO15693 WORKING!")); Serial.println(" ");
}

void loop() {
    esp_task_wdt_reset();

    // Try to read a tag ID (or "get inventory" in ISO15693-speak)
    uint8_t uid[8];
    ISO15693ErrorCode rc = nfc.getInventory(uid);

    if (sensor == nullptr) {
        sensor = std::make_unique<cgm::sensor>();
        sensor->set_UID(uid);
    }
    std::vector<uint8_t> uid_vector(uid, uid + sizeof(uid));
    
    // If the result code was that a card had been read, and is TI tag
    if (rc == ISO15693_EC_OK && uid[6] == cgm::TEXAS_INSTRUMENTS) {
        
        relayState = HIGH;
        digitalWrite(relayPin, relayState);
        
        Serial.println(F("RFID Card Detected!"));

        Serial.print("Tag UID: ");

        // Format each byte as HEX, padded with leading zeros if required
        for (int i = 7; i >= 0; i--) {
            if (uid[i] < 0x10) Serial.print("0");
            Serial.print(uid[i], HEX);
        }
        Serial.println();

        uint8_t buffer[7] = {};
        auto ret = nfc.getPatchInfo(buffer);

        Serial.print("Sensor info ret: ");
        Serial.println(ret);


        /// Get system info for Block Size and Number of Blocks ///
        uint8_t blockSize, numBlocks;
        rc = nfc.getSystemInfo(uid, &blockSize, &numBlocks);

        if (rc != ISO15693_EC_OK) {
            Serial.print(F("Error in getSystemInfo: "));
            Serial.println(rc);
        } else {
            Serial.print(F("System Info retrieved: blockSize="));
            Serial.print(blockSize);
            Serial.print(F(", numBlocks="));
            Serial.println(numBlocks);
        }

        /// Get patch info ///
        // std::vector<uint8_t> patch_info = {0xC5, 0x09, 0x30, 0x01, 0x00, 0x00};
        std::vector<uint8_t> patch_info = {0xC6, 0X09, 0X31, 0X01, 0X16, 0X0A};

        // Check sensor is Libre 2 EU
        if (cgm::get_sensor_type(patch_info) == cgm::sensorType::LIBRE2EU) {
            Serial.println("Sensor is Libre 2 EU");
        } else {
            Serial.println("Sensor is not Libre 2 EU");
        }


        /// Read sensor FRAM and decrypt ///
        std::vector<uint8_t> FRAM_vector;
        FRAM_vector.reserve(43 * blockSize);

        cgm::read_FRAM(nfc, uid_vector, blockSize, FRAM_vector);

        auto decrypted_fram = cgm::decrypt_FRAM(uid_vector, patch_info, FRAM_vector);

        cgm::FRAM_data fram_struct(decrypted_fram);

        // // std::vector<uint8_t> headerVector;
        // // headerVector.reserve(3 * blockSize);
        // // cgm::readHeader(nfc, uid_vector, blockSize, headerVector);

        // // // Print header vector
        // // Serial.println("Header vector:");
        // // for (const auto& byte : headerVector) {
        // //     if (byte < 0x10) Serial.print("0");
        // //     Serial.print(byte, HEX);
        // //     Serial.print(" ");
        // // }

        auto crc = cgm::check_CRC16(std::vector<uint8_t>(decrypted_fram.begin(), decrypted_fram.begin() + 24), 0);
        // auto crc = cgm::check_CRC16(fram_struct.header_crc, 0);
        Serial.print("Header CRC match: ");
        Serial.println(crc != 0);

        auto crc_body = cgm::check_CRC16(std::vector<uint8_t>(decrypted_fram.begin() + 24, decrypted_fram.begin() + 24 + 296), 1);
        Serial.print("Body CRC match: ");
        Serial.println(crc_body != 0);

        auto crc_footer = cgm::check_CRC16(std::vector<uint8_t>(decrypted_fram.begin() + 24 + 296, decrypted_fram.begin() + 24 + 296 + 24), 2);
        Serial.print("Footer CRC match: ");
        Serial.println(crc_footer != 0);


        /// TODO: stuff ///
        if (decrypted_fram.size() >= 318) {
            auto age = (int(decrypted_fram[316]) + int(decrypted_fram[317])) << 8;
            Serial.print("Age: ");
            Serial.println(age);
        } else {
            Serial.println("Error: decrypted_fram size is less than 318");
        }

        auto state = cgm::get_sensor_state(decrypted_fram);
        Serial.print("Sensor state: ");
        Serial.println(cgm::to_string(state).c_str());

        auto region = cgm::get_sensor_region(patch_info);
        Serial.print("Sensor region: ");
        Serial.println(cgm::to_string(region).c_str());

        auto serial = cgm::get_sensor_serial_number(uid_vector, patch_info);
        Serial.print("Sensor serial: ");
        Serial.println(serial.c_str());

        Serial.print("Trend pointer: ");
        Serial.println(fram_struct.trend_index);

        Serial.print("Historic pointer: ");
        Serial.println(fram_struct.historic_index);

        Serial.println(std::format("Trend record {}", fram_struct.trend_index).c_str());
        Serial.print("Error: ");
        Serial.print(fram_struct.trend_records[fram_struct.trend_index - 1].has_error);
        Serial.print(" Negative: ");
        Serial.print(fram_struct.trend_records[fram_struct.trend_index - 1].negative);
        Serial.print(" Raw glucose: ");
        Serial.print(fram_struct.trend_records[fram_struct.trend_index - 1].raw_glucose);
        Serial.print(" Raw temperature: ");
        Serial.print(fram_struct.trend_records[fram_struct.trend_index - 1].raw_temperature);
        Serial.print(" Temperature adjustment: ");
        Serial.println(fram_struct.trend_records[fram_struct.trend_index - 1].temperature_adjustment);

        delay(1000);  
        } else { // If a card is not detected

            if (rc == ISO15693_EC_OK) { // If UID present, but not TI
                Serial.println("Card detected is not Texas Instruments Sensor");
                delay(900);
            } else {
                // The most significant (last) byte of a valid UID should always be 0xE0. e.g. E007C4A509C247A8
                relayState = LOW;
                digitalWrite(relayPin, relayState);
                    
                Serial.println("Card removed from the reader!");
                delay(900);
            }

        #ifdef DEBUG
        Serial.print(F("Error when reading : "));
        // Serial.println(nfc.strerror(rc));
        #endif
    }
}
