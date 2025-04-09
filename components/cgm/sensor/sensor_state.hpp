#ifndef __CGM_SENSOR_STATE_HPP__
#define __CGM_SENSOR_STATE_HPP__

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

/**
 * @brief sensor_state enum class to define sensor states
 * 
 */
enum class sensor_state {
    UNKNOWN,
    NOT_YET_STARTED,
    INITIALISING,
    READY,
    EXPIRED,
    SHUTDOWN,
    FAILURE,
};

/**
 * @brief Get the sensor state object
 * 
 * @param fram the sensor FRAM
 * @return `sensor_state` - the sensor state 
 */
sensor_state get_sensor_state(const std::vector<uint8_t>& fram);

/**
 * @brief Get the sensor state object
 * 
 * @param state_byte the sensor state byte from sensor FRAM[4]
 * @return `sensor_state`- the sensor state
 */
sensor_state get_sensor_state(const uint8_t& state_byte);

/**
 * @brief convert sensor_state to string
 * 
 * @param state the sensor state to convert
 * @return `std::string` - the string representation of the sensor state
 */
std::string to_string(sensor_state state);

} // namespace cgm

#endif // __CGM_SENSOR_STATE_HPP__
