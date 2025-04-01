#include <sensor/sensor_region.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensorRegion get_sensor_region(const std::vector<uint8_t>& patch_info) {
    switch (patch_info[3]) {
        case 0x00:
            return sensorRegion::UNKNOWN;
        case 0x01:
            return sensorRegion::EUROPE;
        case 0x02:
            return sensorRegion::USA;
        case 0x04:
            return sensorRegion::AUSTRALIA_CANADAIAN;
        case 0x08:
            return sensorRegion::EASTERN;
        default:
            return sensorRegion::UNKNOWN;
    }
}

std::string to_string(sensorRegion region) {
    switch (region) {
        case sensorRegion::UNKNOWN:
            return "Unknown";
        case sensorRegion::EUROPE:
            return "European";
        case sensorRegion::USA:
            return "USA";
        case sensorRegion::AUSTRALIA_CANADAIAN:
            return "Australia/Canada";
        case sensorRegion::EASTERN:
            return "Eastern";
        default:
            return "Unknown";
    }
}

} // namespace cgm
