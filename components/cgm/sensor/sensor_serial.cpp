#include <sensor/sensor_serial.hpp>

#include <sensor/sensor_family.hpp>

#include <vector>
#include <cstdint>
#include <string>
#include <sstream>

namespace cgm {

std::string get_sensor_serial_number(const std::vector<uint8_t>& uid, const std::vector<uint8_t>& patch_info) {
    // reverse the UID
    std::vector<uint8_t> reversed_uid(uid.rbegin(), uid.rend());
    
    // remove first two bytes from UID
    std::vector<uint8_t> bytes(reversed_uid.begin() + 2, reversed_uid.end());

    std::vector<uint8_t> five_bits_array;
    five_bits_array.reserve(10);

    five_bits_array.push_back(bytes[0] >> 3);
    five_bits_array.push_back((bytes[0] << 2) + (bytes[1] >> 6));

    five_bits_array.push_back(bytes[1] >> 1);
    five_bits_array.push_back((bytes[1] << 4) + (bytes[2] >> 4));

    five_bits_array.push_back((bytes[2] << 1) + (bytes[3] >> 7));

    five_bits_array.push_back(bytes[3] >> 2);
    five_bits_array.push_back((bytes[3] << 3) + (bytes[4] >> 5));

    five_bits_array.push_back(bytes[4]);

    five_bits_array.push_back(bytes[5] >> 3);
    five_bits_array.push_back(bytes[5] << 2);

    std::stringstream result;

    // add sensor family to start of serial number
    result << (int)get_sensor_family(patch_info);

    for (const auto& byte : five_bits_array) {
        result << serial_lookup[byte & 0x1F]; // mask with 0x1F to get the 5 relevant bits
    }

    return result.str();
}

} // namespace cgm
