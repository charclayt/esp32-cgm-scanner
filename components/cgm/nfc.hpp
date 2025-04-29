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
 * @brief Read the FRAM data from the sensor
 * 
 * @param uid the UID of the sensor
 * @param block_size the size of the blocks to read
 * @param buffer the buffer to fill with the FRAM data
 * @return `ISO15693ErrorCode` - the error code
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
 * @param uid the UID of the sensor
 * @param sensor_info the sensor info
 * @param buffer the buffer to fill with the peripheral MAC address to connect to
 * @return `ISO15693ErrorCode` - the error code
 */
ISO15693ErrorCode initiate_ble_connection(std::vector<uint8_t>& uid, std::vector<uint8_t>& sensor_info, std::vector<uint8_t>& buffer);

private:

uint32_t m_unlock_code = 42;

};

} // namespace cgm

#endif // __CGM_NFC_HPP__
