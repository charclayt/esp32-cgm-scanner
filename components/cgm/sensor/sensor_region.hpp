#ifndef __CGM_SENSOR_REGION_HPP__
#define __CGM_SENSOR_REGION_HPP__

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

/**
 * @brief sensor_region enum class to define sensor regions
 * 
 */
enum class sensor_region {
    UNKNOWN,
    EUROPE,
    USA,
    AUSTRALIA_CANADAIAN,
    EASTERN,
};

/**
 * @brief Get the sensor region object
 * 
 * @param patch_info the sensors patch info
 * @return `sensor_region` - the sensor region
 */
sensor_region get_sensor_region(const std::vector<uint8_t>& patch_info);

/**
 * @brief Get the sensor region object
 * 
 * @param region_byte the sensor region byte from sensor patch_info[3]
 * @return `sensor_region` - the sensor region
 */
sensor_region get_sensor_region(const uint8_t& region_byte);

/**
 * @brief convert sensor_region to string
 * 
 * @param region the sensor region to convert
 * @return `std::string` - the string representation of the sensor region 
 */
std::string to_string(sensor_region region);

} // namespace cgm

#endif // __CGM_SENSOR_REGION_HPP__
