#include <sensor/sensor_family.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

sensor_family get_sensor_family(const std::vector<uint8_t>& patch_info) {
    if (patch_info.size() < 3) {
        return sensor_family::UNKNOWN;
    }

    return get_sensor_family(patch_info[2]);
}

sensor_family get_sensor_family(const uint8_t& family_byte) {
    // shift the family byte to the right by 4 bits
    auto byte_shift = family_byte >> 4;

    switch (byte_shift) {
        case 0:
            return sensor_family::LIBRE1;
        case 1:
            return sensor_family::LIBRE_PRO;
        case 3:
            return sensor_family::LIBRE2;
        case 7:
            return sensor_family::LIBRE_SENSE;
        default:
            return sensor_family::UNKNOWN;
    }
}

std::string to_string(sensor_family family) {
    switch (family) {
        case sensor_family::UNKNOWN:
            return "Unknown";
        case sensor_family::LIBRE1:
            return "Libre 1";
        case sensor_family::LIBRE_PRO:
            return "Libre Pro";
        case sensor_family::LIBRE2:
            return "Libre 2";
        case sensor_family::LIBRE_SENSE:
            return "Libre Sense";
        default:
            return "Unknown";
    }
}

} // namespace cgm
