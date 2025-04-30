#ifndef __CGM_SENSOR_SERIAL_HPP__
#define __CGM_SENSOR_SERIAL_HPP__

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

/**
 * @brief Get the sensor serial number object
 * 
 * @param uid the sensor UID
 * @param patch_info the sensor patch info
 * @return `std::string` - the sensor serial number
 */
std::string get_sensor_serial_number(const std::vector<uint8_t>& uid, const std::vector<uint8_t>& patch_info);

/**
 * @brief The serial lookup table for converting bytes to serial number characters
 * 
 */
const std::vector<std::string> serial_lookup = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                                                "A", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M",
                                                "N", "P", "Q", "R", "T", "U", "V", "W", "X", "Y", "Z" };

} // namespace cgm

#endif // __CGM_SENSOR_SERIAL_HPP__
