#include <decrypt.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

std::vector<uint8_t> decryptFRAM(std::vector<uint8_t>& uid, const std::vector<uint8_t>& info, const std::vector<uint8_t>& encryptedData) {
    std::vector<uint8_t> result;
    result.reserve((sizeof(uint8_t) * 8) * 43);

    for (auto i = 0; i < 43; ++i) {
        auto input = prepareVariables(uid, static_cast<uint16_t>(i), static_cast<uint16_t>(((info[5] << 8) | info[4]) ^ 0x44));
        auto blockKey = processCrypto(input);

        result.push_back(encryptedData[i * 8 + 0] ^ static_cast<uint8_t>(blockKey[0] & 0xFF));
        result.push_back(encryptedData[i * 8 + 1] ^ static_cast<uint8_t>((blockKey[0] >> 8) & 0xFF));
        result.push_back(encryptedData[i * 8 + 2] ^ static_cast<uint8_t>(blockKey[1] & 0xFF));
        result.push_back(encryptedData[i * 8 + 3] ^ static_cast<uint8_t>((blockKey[1] >> 8) & 0xFF));
        result.push_back(encryptedData[i * 8 + 4] ^ static_cast<uint8_t>(blockKey[2] & 0xFF));
        result.push_back(encryptedData[i * 8 + 5] ^ static_cast<uint8_t>((blockKey[2] >> 8) & 0xFF));
        result.push_back(encryptedData[i * 8 + 6] ^ static_cast<uint8_t>(blockKey[3] & 0xFF));
        result.push_back(encryptedData[i * 8 + 7] ^ static_cast<uint8_t>((blockKey[3] >> 8) & 0xFF));
    }

    return result;
}

std::vector<uint16_t> prepareVariables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y) {
    uint16_t s1 = static_cast<uint16_t>(static_cast<uint32_t>((uid[5] << 8) | uid[4]) + static_cast<uint32_t>(x) + static_cast<uint32_t>(y));
    uint16_t s2 = static_cast<uint16_t>(static_cast<uint32_t>((uid[3] << 8) | uid[2]) + static_cast<uint32_t>(keys[2]));
    uint16_t s3 = static_cast<uint16_t>(static_cast<uint32_t>((uid[1] << 8) | uid[0]) + static_cast<uint32_t>(x) * 2);
    uint16_t s4 = 0x241a ^ keys[3];

    return {s1, s2, s3, s4};
}

std::vector<uint16_t> processCrypto(const std::vector<uint16_t>& input) {
    auto op = [](uint16_t value) -> uint16_t {
        uint16_t res = value >> 2; // result does not include these last 2 bits

        if ((value & 1) != 0) { // if last bit is 1
            res = res ^ keys[1];
        }

        if ((value & 2) != 0) { // if second last bit is 1
            res = res ^ keys[0];
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

} // namespace cgm
