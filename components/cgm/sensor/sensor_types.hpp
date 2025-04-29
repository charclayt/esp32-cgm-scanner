#ifndef __CGM_TYPES_HPP__
#define __CGM_TYPES_HPP__

#include <cstdint>
#include <string>
#include <vector>

namespace cgm {

// CGM library constants
const auto TEXAS_INSTRUMENTS = 0x07;

/**
 * @brief sensor_type enum class to define sensor types
 * 
 */
enum class sensor_type {
    UNKNOWN,
    LIBRE1,
    LIBRE2EU,
    LIBRE2CA,
    LIBRE2US,
    LIBRE3,
    LIBRE_PRO_H,
    LIBRE_SENSE,
    LIBREUS14DAY,
    VIRTUAL,
};

/**
 * @brief Get the sensor type, based on given patchInfo
 * 
 * @param patch_info the sensors patch info
 * @return `sensor_type` - the sensor type
 */
sensor_type get_sensor_type(const std::vector<uint8_t>& patch_info);

/**
 * @brief convert sensor_type to string
 * 
 * @param type the sensor type to convert
 * @return `std::string` - the string representation of the sensor type 
 */
std::string to_string(sensor_type type);

} // namespace cgm

#endif // __CGM_TYPES_HPP__
