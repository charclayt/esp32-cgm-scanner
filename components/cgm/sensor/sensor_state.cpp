#include <sensor/sensor_state.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensor_state get_sensor_state(const std::vector<uint8_t>& fram) {
    return get_sensor_state(fram[4]);
}

sensor_state get_sensor_state(const uint8_t& state_byte) {
    switch (state_byte) {
        case 0x01:
            return sensor_state::NOT_YET_STARTED;
        case 0x02:
            return sensor_state::INITIALISING;
        case 0x03:
            return sensor_state::READY;
        case 0x04:
            return sensor_state::EXPIRED;
        case 0x05:
            return sensor_state::SHUTDOWN;
        case 0x06:
            return sensor_state::FAILURE;
        default:
            return sensor_state::UNKNOWN;
    }
}

std::string to_string(sensor_state state) {
    switch (state) {
        case sensor_state::UNKNOWN:
            return "Unknown";
        case sensor_state::NOT_YET_STARTED:
            return "Sensor not yet started";
        case sensor_state::INITIALISING:
            return "Sensor initialising";
        case sensor_state::READY:
            return "Sensor ready";
        case sensor_state::EXPIRED:
            return "Sensor expired";
        case sensor_state::SHUTDOWN:
            return "Sensor shutdown";
        case sensor_state::FAILURE:
            return "Sensor failure";
        default:
            return "Unknown";
    }
}

} // namespace cgm
