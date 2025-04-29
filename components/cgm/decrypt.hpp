#ifndef __CGM_DECRYPT_HPP__
#define __CGM_DECRYPT_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

// /**
//  * @brief 
//  * 
//  */
// static const std::vector<uint16_t> keys = {0xA0C5, 0x6860, 0x0000, 0x14C6};

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

// /**
//  * @brief Derives 4 16-bit values from the given UID and block information, acting like a pseudo-random seed for the decryption
//  * 
//  * @param uid the sensor's UID
//  * @param x the current block index
//  * @param y derived from (uint16_t) sensor_info[5] and sensor_info[4] XORed with 0x44
//  * @return `std::vector<uint16_t>` - the derived 4 16-bit values
//  */
// std::vector<uint16_t> prepare_variables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y);

// /**
//  * @brief Transforms the output from `prepare_variables` into 4 "decryption words"
//  * 
//  * @param input the 4 16-bit values derived from `prepare_variables`
//  * @return `std::vector<uint16_t>` - the transformed 4 16-bit values
//  */
// std::vector<uint16_t> process_crypto(const std::vector<uint16_t>& input);

// /**
//  * @brief 
//  * 
//  */
// std::vector<uint8_t> useful_function(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y);

} // namespace cgm

#endif // __CGM_DECRYPT_HPP__
