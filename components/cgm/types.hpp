#ifndef CGM_TYPES_H
#define CGM_TYPES_H

#include <vector>
#include <cstdint>

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
 * @param patchInfo the info from sending '0xA1' command to sensor
 * @return `sensorType` - the sensor type
 */
sensorType getSensorType(const std::vector<uint8_t>& patchInfo);

} // namespace cgm

#endif // CGM_TYPES_H
