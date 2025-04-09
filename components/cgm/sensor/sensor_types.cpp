#include <sensor/sensor_types.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensor_type get_sensor_type(const std::vector<uint8_t>& patch_info) {
    switch(patch_info[0]) {
    case 0xDF:
    case 0xA2:
        return sensor_type::LIBRE1;
    case 0xE5:
    case 0xE6:
        return sensor_type::LIBREUS14DAY;
    case 0x70:
        return sensor_type::LIBRE_PRO_H;
    case 0x9D:
    case 0xC5:
    case 0xC6:
        return sensor_type::LIBRE2EU;
    case 0x76:
        return patch_info[3] == 0x02 ? sensor_type::LIBRE2US
           : patch_info[3] == 0x04 ? sensor_type::LIBRE2CA
           : patch_info[2] >> 4 == 7 ? sensor_type::LIBRE_SENSE
           : sensor_type::UNKNOWN;
    default:
        if (patch_info.size() > 6) { // Libre 3 NFC 0xA1 command returns 35 or 28 bytes
            return sensor_type::LIBRE3;
        } else {
            return sensor_type::UNKNOWN;
        }
    }
}

std::string to_string(sensor_type type) {
    switch (type) {
    case sensor_type::UNKNOWN:
        return "Unknown";
    case sensor_type::LIBRE1:
        return "Libre 1";
    case sensor_type::LIBRE2EU:
        return "Libre 2 EU";
    case sensor_type::LIBRE2CA:
        return "Libre 2 CA";
    case sensor_type::LIBRE2US:
        return "Libre 2 US";
    case sensor_type::LIBRE3:
        return "Libre 3";
    case sensor_type::LIBRE_PRO_H:
        return "Libre Pro/H";
    case sensor_type::LIBRE_SENSE:
        return "Libre Sense";
    case sensor_type::LIBREUS14DAY:
        return "Libre US 14 day";
    case sensor_type::VIRTUAL:
        return "Virtual";
    default:
        return "Unknown";
    }
}

} // namespace cgm
