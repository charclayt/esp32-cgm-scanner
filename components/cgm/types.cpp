#include <types.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

sensorType getSensorType(const std::vector<uint8_t>& patchInfo) {
    switch(patchInfo[0]) {
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
        return patchInfo[3] == 0x02 ? sensorType::LIBRE2US
           : patchInfo[3] == 0x04 ? sensorType::LIBRE2CA
           : patchInfo[2] >> 4 == 7 ? sensorType::LIBRE_SENSE
           : sensorType::UNKNOWN;
    default:
        if (patchInfo.size() > 6) { // Libre 3 NFC 0xA1 command returns 35 or 28 bytes
            return sensorType::LIBRE3;
        } else {
            return sensorType::UNKNOWN;
        }
    }
}

} // namespace cgm
