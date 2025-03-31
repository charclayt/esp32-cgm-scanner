#include "Arduino.h"

#include "PN5180.h"
#include "PN5180ISO15693.h"

// CGM includes
#include "decrypt.hpp"
#include "types.hpp"
#include "nfc.hpp"
#include "sensor.hpp"
#include "crc.hpp"

#include <esp_task_wdt.h>

#include <stdio.h>
#include <memory>


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
    Serial.begin(115200);
    pinMode(relayPin, OUTPUT);
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

        // /// Read FRAM Blocks ///
        // uint8_t readBuffer[blockSize];
        // uint8_t numFRAMBlocks = 43;

        // std::vector<uint8_t> dataVector;
        // dataVector.reserve(numFRAMBlocks * blockSize);

        // Read header of FRAM

        std::vector<uint8_t> uid_vector(uid, uid + sizeof(uid));
        std::vector<uint8_t> patch_info = {0xC5, 0x09, 0x30, 0x01, 0x00, 0x00};

        std::vector<uint8_t> FRAM_vector;
        FRAM_vector.reserve(43 * blockSize);
        cgm::readFRAM(nfc, uid_vector, blockSize, FRAM_vector);

        auto decrypted_fram = cgm::decryptFRAM(uid_vector, patch_info, FRAM_vector);

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
        Serial.print("Header CRC match: ");
        Serial.println(crc);

        auto crc_body = cgm::check_CRC16(std::vector<uint8_t>(decrypted_fram.begin() + 24, decrypted_fram.begin() + 24 + 296), 1);
        Serial.print("Body CRC match: ");
        Serial.println(crc_body);

        auto crc_footer = cgm::check_CRC16(std::vector<uint8_t>(decrypted_fram.begin() + 24 + 296, decrypted_fram.begin() + 24 + 296 + 24), 2);
        Serial.print("Footer CRC match: ");
        Serial.println(crc_footer);

        // Read header of FRAM


        // for (int num = 0; num < numFRAMBlocks; num++) {
        //     rc = nfc.readSingleBlock(uid, num, readBuffer, blockSize);

        //     if (rc != ISO15693_EC_OK) {
        //         Serial.print(F("Error in readSingleBlock #"));
        //         Serial.print(num);
        //         Serial.print(": ");
        //         Serial.println(rc);
        //     } else {
        //         // Output block number in hex padded with leading zeros if required
        //         Serial.print(F("Block "));
        //         if (num < 0x10) Serial.print("0");
        //         Serial.print(num, HEX);

        //         // Spacing
        //         Serial.print("   ");

        //         // Output block data in hex padded with leading zeros if required
        //         for (int i = 0; i < blockSize; i++) {
        //             // Add data to vector
        //             dataVector.push_back(readBuffer[i]);

        //             if (readBuffer[i] < 0x10) Serial.print("0");
        //             Serial.print(readBuffer[i], HEX);
        //             Serial.print(" ");
        //         }

        //         // Spacing
        //         Serial.print("   ");

        //         // Output block data in char
        //         for (int i = 0; i < blockSize; i++) {
        //             if (isprint(readBuffer[i])) {
        //                 Serial.print((char)readBuffer[i]);
        //             } else {
        //                 Serial.print(".");
        //             }
        //         }
        //     }
        //     Serial.println();
        // }


        // /// Get patch info ///

        // // uint8_t buffer[7];
        // // uint8_t buffer[7] = {};
        // // rc = nfc.getSensorInfo(uid, buffer);

        // // Serial.print("Sensor info rc: ");
        // // Serial.println(rc);

        // // Throw away first byte
        // std::vector<uint8_t> patchInfo(buffer + 1, buffer + sizeof(buffer));

        // // std::vector<uint8_t> patchInfo = {0xC5, 0x09, 0x30, 0x01, 0x00, 0x00};

        // Serial.print("Buffer: ");
        // for (const auto& byte : patchInfo) {
        //     if (byte < 0x10) Serial.print("0");
        //     Serial.print(byte, HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();


        // /// Decrypt FRAM and check CRC ///
        // std::vector<uint8_t> decryptedFRAM = cgm::decryptFRAM(uid, patchInfo, dataVector);

        // // if (crc16(std::vector<uint8_t>(decryptedFRAM.begin() + 2, decryptedFRAM.begin() + 2 + (0x0B * 2))) == (decryptedFRAM[0] | decryptedFRAM[1] << 8)) {
        // //   Serial.println("Header CRC OK");
        // // } else {
        // //   Serial.println("Header CRC ERROR");
        // // }

        // if (decryptedFRAM.size() >= 318) {
        //     auto age = (int(decryptedFRAM[316]) + int(decryptedFRAM[317])) << 8;
        //     Serial.print("Age: ");
        //     Serial.println(age);
        // } else {
        //     Serial.println("Error: decryptedFRAM size is less than 318");
        // }

        // // Check sensor is Libre 2 EU
        // if (cgm::getSensorType(patchInfo) == cgm::sensorType::LIBRE2EU) {
        //     Serial.println("Sensor is Libre 2 EU");
        // } else {
        //     Serial.println("Sensor is not Libre 2 EU");
        // }

        // // Check header CRC
        // // print_crc("Header CRC", std::vector<uint8_t>(decryptedFRAM.begin(), decryptedFRAM.begin() + 2),
        // //                         std::vector<uint8_t>(decryptedFRAM.begin() + 2, decryptedFRAM.begin() + 2 + (0x0B * 2)));

        // // // Check record CRC
        // // print_crc("Record CRC", std::vector<uint8_t>(dataVector.begin() + 24, dataVector.begin() + 26),
        // //                         std::vector<uint8_t>(dataVector.begin() + 26, dataVector.begin() + 26 + (0x93 * 2)));

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
