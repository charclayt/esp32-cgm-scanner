#include <sensor/sensor_types.hpp>

#include <vector>
#include <cstdint>
#include <string>

namespace cgm {

sensorType get_sensor_type(const std::vector<uint8_t>& patch_info) {
    switch(patch_info[0]) {
    case 0xDF:
    case 0xA2:
        return sensorType::LIBRE1;
    case 0xE5:
    case 0xE6:
        return sensorType::LIBREUS14DAY;
    case 0x70:
        return sensorType::LIBRE_PRO_H;
    case 0x9D:
    case 0xC5:
    case 0xC6:
        return sensorType::LIBRE2EU;
    case 0x76:
        return patch_info[3] == 0x02 ? sensorType::LIBRE2US
           : patch_info[3] == 0x04 ? sensorType::LIBRE2CA
           : patch_info[2] >> 4 == 7 ? sensorType::LIBRE_SENSE
           : sensorType::UNKNOWN;
    default:
        if (patch_info.size() > 6) { // Libre 3 NFC 0xA1 command returns 35 or 28 bytes
            return sensorType::LIBRE3;
        } else {
            return sensorType::UNKNOWN;
        }
    }
}

std::string to_string(sensorType type) {
    switch (type) {
    case sensorType::UNKNOWN:
        return "Unknown";
    case sensorType::LIBRE1:
        return "Libre 1";
    case sensorType::LIBRE2EU:
        return "Libre 2 EU";
    case sensorType::LIBRE2CA:
        return "Libre 2 CA";
    case sensorType::LIBRE2US:
        return "Libre 2 US";
    case sensorType::LIBRE3:
        return "Libre 3";
    case sensorType::LIBRE_PRO_H:
        return "Libre Pro/H";
    case sensorType::LIBRE_SENSE:
        return "Libre Sense";
    case sensorType::LIBREUS14DAY:
        return "Libre US 14 day";
    case sensorType::VIRTUAL:
        return "Virtual";
    default:
        return "Unknown";
    }
}

} // namespace cgm
