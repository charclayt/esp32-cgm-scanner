#ifndef __CGM_DECRYPT_HPP__
#define __CGM_DECRYPT_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief 
 * 
 */
static const std::vector<uint16_t> keys = {0xA0C5, 0x6860, 0x0000, 0x14C6};

/**
 * @brief Decrypts the given FRAM data using the sensor's UID and patch info
 * 
 * @param uid the sensor's UID
 * @param info the sensor's patch info
 * @param encryptedData the encrypted FRAM data
 * @return `std::vector<uint8_t>` - the decrypted FRAM data
 */
std::vector<uint8_t> decrypt_FRAM(std::vector<uint8_t>& uid, const std::vector<uint8_t>& info, const std::vector<uint8_t>& encryptedData);

/**
 * @brief 
 * 
 * @param uid the sensor's UID
 * @param x 
 * @param y 
 * @return std::vector<uint16_t> 
 */
std::vector<uint16_t> prepareVariables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y);

/**
 * @brief 
 * 
 * @param input 
 * @return std::vector<uint16_t> 
 */
std::vector<uint16_t> processCrypto(const std::vector<uint16_t>& input);

} // namespace cgm

#endif // __CGM_DECRYPT_HPP__
