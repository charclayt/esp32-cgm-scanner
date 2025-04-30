#ifndef __CGM_DECRYPT_HPP__
#define __CGM_DECRYPT_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief Decrypts the given FRAM data using the sensor's UID and patch info
 * 
 * @param uid the sensor's UID
 * @param info the sensor's patch info
 * @param encrypted_data the encrypted FRAM data
 * @return `std::vector<uint8_t>` - the decrypted FRAM data if CRC check passes, otherwise an empty vector
 */
std::vector<uint8_t> decrypt_FRAM(std::vector<uint8_t>& uid, const std::vector<uint8_t>& info, const std::vector<uint8_t>& encrypted_data);


/**
 * @brief Decrypts the received BLE data using the sensor's UID
 * 
 * @param uid the sensor's UID
 * @param encrypted_data the encrypted BLE data
 * @return `std::vector<uint8_t>` - the decrypted BLE data
 */
std::vector<uint8_t> decrypt_BLE(std::vector<uint8_t>& uid, const std::vector<uint8_t>& encrypted_data);

} // namespace cgm

#endif // __CGM_DECRYPT_HPP__
