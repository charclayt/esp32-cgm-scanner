#include <sensor/sensor_region.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensor_region get_sensor_region(const std::vector<uint8_t>& patch_info) {
    if (patch_info.size() < 4) {
        return sensor_region::UNKNOWN;
    }

    return get_sensor_region(patch_info[3]);
}

sensor_region get_sensor_region(const uint8_t& region_byte) {
    switch (region_byte) {
        case 0x00:
            return sensor_region::UNKNOWN;
        case 0x01:
            return sensor_region::EUROPE;
        case 0x02:
            return sensor_region::USA;
        case 0x04:
            return sensor_region::AUSTRALIA_CANADAIAN;
        case 0x08:
            return sensor_region::EASTERN;
        default:
            return sensor_region::UNKNOWN;
    }
}

std::string to_string(sensor_region region) {
    switch (region) {
        case sensor_region::UNKNOWN:
            return "Unknown";
        case sensor_region::EUROPE:
            return "European";
        case sensor_region::USA:
            return "USA";
        case sensor_region::AUSTRALIA_CANADAIAN:
            return "Australia/Canada";
        case sensor_region::EASTERN:
            return "Eastern";
        default:
            return "Unknown";
    }
}

} // namespace cgm
