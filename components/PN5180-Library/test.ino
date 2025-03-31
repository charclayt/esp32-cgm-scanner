#include <PN5180.h>
#include <PN5180ISO15693.h>
#include <esp_task_wdt.h>

#include <vector>
#include <bitset>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#define WDT_TIMEOUT 2 //2 seconds WDT

// ESP-32 <-> PN5180 pinout mapping
#define PN5180_NSS  12  // GPIO12
#define PN5180_BUSY 13  // GPIO13
#define PN5180_RST  14  // GPIO14

const int relayPin =  25 ;  // GPIO25
int relayState = LOW;   // relayState used to set the relayPin

PN5180ISO15693 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

// ToDo: not sure if key is supposed to be hardcoded...
static const std::vector<uint16_t> key = {0xA0C5, 0x6860, 0x0000, 0x14C6};

std::vector<uint16_t> prepareVariables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
  uint16_t s1 = static_cast<uint16_t>(static_cast<uint32_t>((uid[5] << 8) | uid[4]) + static_cast<uint32_t>(x) + static_cast<uint32_t>(y));
  uint16_t s2 = static_cast<uint16_t>(static_cast<uint32_t>((uid[3] << 8) | uid[2]) + static_cast<uint32_t>(key[2]));
  uint16_t s3 = static_cast<uint16_t>(static_cast<uint32_t>((uid[1] << 8) | uid[0]) + static_cast<uint32_t>(x) * 2);
  uint16_t s4 = 0x241a ^ key[3];

  return {s1, s2, s3, s4};
}

std::vector<uint16_t> processCrypto(const std::vector<uint16_t>& input) {
  auto op = [](uint16_t value) -> uint16_t {
    uint16_t res = value >> 2; // result does not include these last 2 bits

    if (value & 1) { // if last bit is 1
      res = res ^ key[1];
    }

    if (value & 2) { // if second last bit is 1
      res = res ^ key[0];
    }

    return res;
  };

  uint16_t r0 = op(input[0]) ^ input[3];
  uint16_t r1 = op(r0) ^ input[2];
  uint16_t r2 = op(r1) ^ input[1];
  uint16_t r3 = op(r2) ^ input[0];
  uint16_t r4 = op(r3);
  uint16_t r5 = op(r4 ^ r0);
  uint16_t r6 = op(r5 ^ r1);
  uint16_t r7 = op(r6 ^ r2);

  uint16_t f1 = r0 ^ r4;
  uint16_t f2 = r1 ^ r5;
  uint16_t f3 = r2 ^ r6;
  uint16_t f4 = r3 ^ r7;

  return {f4, f3, f2, f1};
}

// ToDo: Requires method to get sensor info
std::vector<uint8_t> decryptFRAM(const uint8_t* uid, const std::vector<uint8_t>& info, const std::vector<uint8_t>& encryptedData) {
  std::vector<uint8_t> result;
  result.reserve(43 * 8);

  for (auto i = 0; i < 43; ++i) {
    auto input = prepareVariables(std::vector<uint8_t>(uid), static_cast<uint16_t>(i), static_cast<uint16_t>(((info[5] << 8) | info[4]) ^ 0x44));
    auto blockKey = processCrypto(input);

    result.push_back(encryptedData[i * 8 + 0] ^ static_cast<uint8_t>(blockKey[0] & 0xFF));
    result.push_back(encryptedData[i * 8 + 1] ^ static_cast<uint8_t>(blockKey[0] >> 8 & 0xFF));
    result.push_back(encryptedData[i * 8 + 2] ^ static_cast<uint8_t>(blockKey[1] & 0xFF));
    result.push_back(encryptedData[i * 8 + 3] ^ static_cast<uint8_t>(blockKey[1] >> 8 & 0xFF));
    result.push_back(encryptedData[i * 8 + 4] ^ static_cast<uint8_t>(blockKey[2] & 0xFF));
    result.push_back(encryptedData[i * 8 + 5] ^ static_cast<uint8_t>(blockKey[2] >> 8 & 0xFF));
    result.push_back(encryptedData[i * 8 + 6] ^ static_cast<uint8_t>(blockKey[3] & 0xFF));
    result.push_back(encryptedData[i * 8 + 7] ^ static_cast<uint8_t>(blockKey[3] >> 8 & 0xFF));
  }

  return result;
}

uint16_t computeSensorCrc(const std::vector<uint8_t>& data) {
  uint16_t crc = 0x0000FFFF;
  auto datalen = data.size();

  for (auto i = 0; i < datalen; ++i) {
    uint8_t rev = std::bitset<8>(data[i]).to_ulong();
    rev = ((rev * 0x0202020202ULL & 0x010884422010ULL) % 0x3ff); // reverse bits

    crc = ((crc >> 8) & 0x0000FFFF) | ((crc << 8) & 0x0000FFFF);
    crc = crc ^ rev;
    crc = crc ^ (((crc & 0xFF) >> 4) & 0x0000FFFF);
    crc = crc ^ ((crc << 12) & 0x0000FFFF);
    crc = crc ^ (((crc & 0xFF) << 5) & 0x0000FFFF);
  }

  return crc;
}

std::string toHexString(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (const auto& byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

void print_crc(const std::string& label, const std::vector<uint8_t>& crc_read, const std::vector<uint8_t>& data) {
    uint16_t crc_computed_value = computeSensorCrc(data);
    std::vector<uint8_t> crc_computed = { static_cast<uint8_t>(crc_computed_value & 0xFF), static_cast<uint8_t>((crc_computed_value >> 8) & 0xFF) };

    std::string crc_read_hex = toHexString(crc_read);
    std::string crc_computed_hex = toHexString(crc_computed);

    Serial.print(label.c_str());
    Serial.print(": read=");
    Serial.print(crc_read_hex.c_str());
    Serial.print(" computed=");
    Serial.print(crc_computed_hex.c_str());

    if (crc_read_hex == crc_computed_hex) {
        Serial.println("  OK");
    } else {
      Serial.println("  ERROR: CRC does not match");
    }
}

void printDataVector(const std::vector<uint8_t>& dataVector) {
    Serial.print("Data Vector: ");
    for (const auto& byte : dataVector) {
        if (byte < 0x10) {
            Serial.print("0");
        }
        Serial.print(byte, HEX);
        Serial.print(" ");
    }
    Serial.println();
}


void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  delay(500);

  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = WDT_TIMEOUT * 2000,
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

void loop(void) {

  uint8_t uid[8];
  
  esp_task_wdt_reset();

  // Try to read a tag ID (or "get inventory" in ISO15693-speak)
  ISO15693ErrorCode rc = nfc.getInventory(uid);
  
  // If the result code was that a card had been read
  if (rc == ISO15693_EC_OK) {
    
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


    // Get system info for Block Size and Number of Blocks
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

    // Read FRAM Blocks
    uint8_t readBuffer[blockSize];
    uint8_t numFRAMBlocks = 44;

    std::vector<uint8_t> dataVector;
    dataVector.reserve(numFRAMBlocks * blockSize);

    for (int num = 0; num < numFRAMBlocks; num++) {
      rc = nfc.readSingleBlock(uid, num, readBuffer, blockSize);

      if (rc != ISO15693_EC_OK) {
        Serial.print(F("Error in readSingleBlock #"));
        Serial.print(num);
        Serial.print(": ");
        Serial.println(rc);
      } else {
        // Output block number in hex padded with leading zeros if required
        Serial.print(F("Block "));
        if (num < 0x10) Serial.print("0");
        Serial.print(num, HEX);

        // Spacing
        Serial.print("   ");

        // Output block data in hex padded with leading zeros if required
        for (int i = 0; i < blockSize; i++) {
          // Add data to vector
          dataVector.push_back(readBuffer[i]);

          if (readBuffer[i] < 0x10) Serial.print("0");
          Serial.print(readBuffer[i], HEX);
          Serial.print(" ");
        }

        // Spacing
        Serial.print("   ");

        // Output block data in char
        for (int i = 0; i < blockSize; i++) {
          if (isprint(readBuffer[i])) {
            Serial.print((char)readBuffer[i]);
          } else {
            Serial.print(".");
          }
        }
      }
      Serial.println();
    }

    uint8_t buffer[7];
    rc = nfc.getSensorInfo(uid, buffer);

    Serial.print("Sensor info rc: ");
    Serial.println(rc);

    // if (rc == ISO15693_EC_OK) {
      // Throw away first byte
    std::vector<uint8_t> patchInfo(buffer + 1, buffer + blockSize);
    // }

    Serial.print("SIZE: ");
    Serial.print(sizeof(buffer));
    Serial.print(" ");
    Serial.println(sizeof(patchInfo));

    Serial.print("Buffer: ");
    for (const auto& byte : patchInfo) {
        if (byte < 0x10) Serial.print("0");
        Serial.print(byte, HEX);
        Serial.print(" ");
    }
    Serial.println();

    std::vector<uint8_t> decryptedFRAM = decryptFRAM(uid, patchInfo, dataVector);

    // Check header CRC
    print_crc("Header CRC", std::vector<uint8_t>(decryptedFRAM.begin(), decryptedFRAM.begin() + 2),
                            std::vector<uint8_t>(decryptedFRAM.begin() + 2, decryptedFRAM.begin() + 2 + (0x0B * 2)));

    // Check record CRC
    print_crc("Record CRC", std::vector<uint8_t>(dataVector.begin() + 24, dataVector.begin() + 26),
                            std::vector<uint8_t>(dataVector.begin() + 26, dataVector.begin() + 26 + (0x93 * 2)));

    delay(1000);  
    }
  
  // If a card is not detected
  else {
    // The most significant (last) byte of a valid UID should always be 0xE0. e.g. E007C4A509C247A8
    if (!rc == ISO15693_EC_OK) {
      
      relayState = LOW;
      digitalWrite(relayPin, relayState);
        
      Serial.println("Card removed from the reader!");
      delay(900);  
    }

    #ifdef DEBUG
    Serial.print(F("Error when reading : "));
    Serial.println(nfc.strerror(rc));
    #endif
  }

}
