#include <sensor/sensor_state.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensorState get_sensor_state(const std::vector<uint8_t>& fram) {
    return get_sensor_state(fram[4]);
}

sensorState get_sensor_state(const uint8_t& state_byte) {
    switch (state_byte) {
        case 0x01:
            return sensorState::NOT_YET_STARTED;
        case 0x02:
            return sensorState::INITIALISING;
        case 0x03:
            return sensorState::READY;
        case 0x04:
            return sensorState::EXPIRED;
        case 0x05:
            return sensorState::SHUTDOWN;
        case 0x06:
            return sensorState::FAILURE;
        default:
            return sensorState::UNKNOWN;
    }
}

std::string to_string(sensorState state) {
    switch (state) {
        case sensorState::UNKNOWN:
            return "Unknown";
        case sensorState::NOT_YET_STARTED:
            return "Sensor not yet started";
        case sensorState::INITIALISING:
            return "Sensor initialising";
        case sensorState::READY:
            return "Sensor ready";
        case sensorState::EXPIRED:
            return "Sensor expired";
        case sensorState::SHUTDOWN:
            return "Sensor shutdown";
        case sensorState::FAILURE:
            return "Sensor failure";
        default:
            return "Unknown";
    }
}

} // namespace cgm
