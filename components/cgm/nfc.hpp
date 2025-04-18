#ifndef __CGM_NFC_HPP__
#define __CGM_NFC_HPP__

#include <vector>
#include <cstdint>

#include "PN5180ISO15693.h"

namespace cgm {

class CGM_NFC : public PN5180ISO15693 {

public:

using PN5180ISO15693::PN5180ISO15693;

/**
 * @brief 
 * 
 * @param uid 
 * @param block_size 
 * @param buffer 
 */
ISO15693ErrorCode read_FRAM(uint8_t* uid, const uint8_t block_size, std::vector<uint8_t>& buffer);

/**
 * @brief Get the sensor info via NFC custom command '0xA1'
 * 
 * @param buffer the buffer to fill with the sensor info
 * @return `ISO15693ErrorCode` - the error code
 */
ISO15693ErrorCode get_sensor_info(std::vector<uint8_t>& buffer);

/**
 * @brief Initiate the BLE connection with the sensor via NFC custom command '0xA1, 0x1e'
 * 
 * @param random_number a random number that acts as the secret key
 * @param integrity_code part of the secret key, compiled of the first 2 bytes of random number and bytes 4 + 5 of the sensor info
 * @return `ISO15693ErrorCode` - the error code
 */
ISO15693ErrorCode initiate_ble_connection(uint8_t* random_number, uint8_t* integrity_code);

};

} // namespace cgm

#endif // __CGM_NFC_HPP__
