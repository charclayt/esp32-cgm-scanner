#ifndef __CGM_SENSOR_REGION_HPP__
#define __CGM_SENSOR_REGION_HPP__

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

/**
 * @brief sensorRegion enum class to define sensor regions
 * 
 */
enum class sensorRegion {
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
 * @return `sensorRegion` - the sensor region
 */
sensorRegion get_sensor_region(const std::vector<uint8_t>& patch_info);

/**
 * @brief convert sensorRegion to string
 * 
 * @param region the sensor region to convert
 * @return `std::string` - the string representation of the sensor region 
 */
std::string to_string(sensorRegion region);

} // namespace cgm

#endif // __CGM_SENSOR_REGION_HPP__
