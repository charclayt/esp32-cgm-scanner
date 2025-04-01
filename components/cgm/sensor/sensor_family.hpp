#ifndef __CGM_SENSOR_FAMILY_HPP__
#define __CGM_SENSOR_FAMILY_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief sensorFamily enum class to define sensor families
 * 
 */
enum class sensorFamily {
    UNKNOWN,
    LIBRE1 = 0,
    LIBRE_PRO = 1,
    LIBRE2 = 3,
    LIBRE_SENSE = 7,
};

/**
 * @brief Get the sensor family object
 * 
 * @param patch_info the sensor patch info
 * @return `sensorFamily` - the sensor family
 */
sensorFamily get_sensor_family(const std::vector<uint8_t>& patch_info);

/**
 * @brief Get the sensor family object
 * 
 * @param family_byte the sensor family byte from sensor patch_info[2]
 * @return `sensorFamily` - the sensor family 
 */
sensorFamily get_sensor_family(const uint8_t& family_byte);

} // namespace cgm

#endif // __CGM_SENSOR_FAMILY_HPP__
