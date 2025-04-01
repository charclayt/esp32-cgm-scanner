#include <sensor/sensor_family.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

sensorFamily get_sensor_family(const std::vector<uint8_t>& patch_info) {
    if (patch_info.size() < 3) {
        return sensorFamily::UNKNOWN;
    }

    return get_sensor_family(patch_info[2]);
}

sensorFamily get_sensor_family(const uint8_t& family_byte) {
    // shift the family byte to the right by 4 bits
    auto byte_shift = family_byte >> 4;

    switch (byte_shift) {
        case 0:
            return sensorFamily::LIBRE1;
        case 1:
            return sensorFamily::LIBRE_PRO;
        case 3:
            return sensorFamily::LIBRE2;
        case 7:
            return sensorFamily::LIBRE_SENSE;
        default:
            return sensorFamily::UNKNOWN;
    }
}

} // namespace cgm
