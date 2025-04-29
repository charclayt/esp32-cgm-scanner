#ifndef __CGM_COMMON_HPP__
#define __CGM_COMMON_HPP__

#include <cstdint>
#include <vector>

namespace cgm {

const auto FRAM_SIZE_BLOCKS = 43;
const auto FRAM_RECORD_SIZE_BYTES = 6;

const auto NUM_FRAM_TREND_RECORDS = 16;
const auto NUM_FRAM_HISTORIC_RECORDS = 32;

const auto BLE_SIZE_BYTES = 43;
const auto BLE_RECORD_SIZE_BYTES = 4;

/**
 * @brief 
 * 
 */
static const std::vector<uint16_t> keys = {0xA0C5, 0x6860, 0x0000, 0x14C6};

int read_bits(const std::vector<uint8_t>& buffer, int byte_offset, int bit_offset, int bit_count);

/**
 * @brief Derives 4 16-bit values from the given UID and block information, acting like a pseudo-random seed for the decryption
 * 
 * @param uid the sensor's UID
 * @param x the current block index
 * @param y derived from (uint16_t) sensor_info[5] and sensor_info[4] XORed with 0x44
 * @return `std::vector<uint16_t>` - the derived 4 16-bit values
 */
std::vector<uint16_t> prepare_variables(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y);

/**
 * @brief Transforms the output from `prepare_variables` into 4 "decryption words"
 * 
 * @param input the 4 16-bit values derived from `prepare_variables`
 * @return `std::vector<uint16_t>` - the transformed 4 16-bit values
 */
std::vector<uint16_t> process_crypto(const std::vector<uint16_t>& input);

/**
 * @brief 
 * 
 */
std::vector<uint8_t> useful_function(const std::vector<uint8_t>& uid, uint16_t x, uint16_t y);
} // namespace cgm

#endif // __CGM_COMMON_HPP__
