#ifndef __CGM_SENSOR_FAMILY_HPP__
#define __CGM_SENSOR_FAMILY_HPP__

#include <cstdint>
#include <string>
#include <vector>

namespace cgm {

/**
 * @brief sensor_family enum class to define sensor families
 * 
 */
enum class sensor_family {
    UNKNOWN = -1,
    LIBRE1 = 0,
    LIBRE_PRO = 1,
    LIBRE2 = 3,
    LIBRE_SENSE = 7,
};

/**
 * @brief Get the sensor family object
 * 
 * @param patch_info the sensor patch info
 * @return `sensor_family` - the sensor family
 */
sensor_family get_sensor_family(const std::vector<uint8_t>& patch_info);

/**
 * @brief Get the sensor family object
 * 
 * @param family_byte the sensor family byte from sensor patch_info[2]
 * @return `sensor_family` - the sensor family 
 */
sensor_family get_sensor_family(const uint8_t& family_byte);

/**
 * @brief Convert sensor_family to string
 * 
 * @param family the sensor family to convert
 * @return `std::string` - the string representation of the sensor family 
 */
std::string to_string(sensor_family family);

} // namespace cgm

#endif // __CGM_SENSOR_FAMILY_HPP__
