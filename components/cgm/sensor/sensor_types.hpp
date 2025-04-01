#ifndef __CGM_TYPES_HPP__
#define __CGM_TYPES_HPP__

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

// CGM library constants
const auto TEXAS_INSTRUMENTS = 0x07;

/**
 * @brief sensorType enum class to define sensor types
 * 
 */
enum class sensorType {
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
 * @return `sensorType` - the sensor type
 */
sensorType get_sensor_type(const std::vector<uint8_t>& patch_info);

/**
 * @brief convert sensorType to string
 * 
 * @param type the sensor type to convert
 * @return `std::string` - the string representation of the sensor type 
 */
std::string to_string(sensorType type);

} // namespace cgm

#endif // __CGM_TYPES_HPP__
