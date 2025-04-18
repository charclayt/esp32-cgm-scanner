#include <record.hpp>

#include <common.hpp>
#include <sensor/factory_calibration.hpp>

#include <algorithm>
#include <cstdint>
#include <vector>

namespace cgm {

uint8_t reverse_bits(uint8_t input) {
    uint8_t output = input;
    for (int i = sizeof(input) * 8 - 1; i; --i) {
        output <<= 1;
        input >>= 1;
        output |= input & 1;
    }
    return output;
}

/*
(pre-reversal) FRAM record layout:

gggg gggg gggg ----
---- ---- -ett tttt
tttt ttaa aaaa aaan

g = raw glucose
e = has error
t = raw temperature
a = temperature adjustment
n = negative

(post-reversal) FRAM record layout:

naaa aaaa aatt tttt
tttt tte- ---- ----
---- gggg gggg gggg
*/

fram_record::fram_record(const std::shared_ptr<FactoryCalibration> calibration, const std::vector<uint8_t> record) {
    // Check if FRAM record is valid (6 bytes)
    if (record.size() != FRAM_RECORD_SIZE_BYTES) {
        has_error = true;
        negative = (temperature_adjustment = (raw_temperature = (raw_glucose = 0)));
        return;
    }

    // TODO: could just pass in copy of record rather than reference
    auto input = record;

    // reverse the bytes in the record
    // std::reverse(input.begin(), input.end());

    // reverse the bits in each byte
    // for (auto& byte : input) {
    //     byte = reverse_bits(byte);
    // }

    // negative bit is now the first bit (bit 0)
    // negative = (input[0] >> 7) & 0x01;
    
    // negative bit is the final bit of the final byte (bit 47)
    negative = readBits(input, 0, 0x2F, 0x1);

    // temperature_adjustment bits are now the final 7 bits of byte 0 and the first 2 bits of byte 1 (bits 1-9)
    // temperature_adjustment = (((input[0] & 0x7F) << 2) | (input[1] >> 6)) << 2;

    // temperature_adjustment bits are the final 2 bits of byte 4 and the first 7 bits of byte 5 (bits 39-47)
    temperature_adjustment = readBits(input, 0, 0x26, 0x9) << 2;
    if (negative != 0) {
        temperature_adjustment = -temperature_adjustment;
    }

    // raw_temperature bits are now the final 6 bits of the byte 1 and the first 6 bits of byte 2 (bits 10-21)
    // raw_temperature = (((input[1] & 0x3F) << 6) | (input[2] >> 2)) << 2;

    // raw_temperature bits are the final 6 bits of byte 3 and the first 6 bits of byte 4 (bits 27-38)
    raw_temperature = readBits(input, 0, 0x1A, 0xC) << 2;

    // has_error bit is now the 7th bit of byte 2 (bit 22)
    // has_error = (input[2] >> 1) & 0x01;

    // has_error bit is the 2nd bit of byte 3 (bit 26)
    has_error = readBits(input, 0, 0x19, 0x1) != 0;

    // raw_glucose bits are now the final byte and the final 4 bits of byte 4 (bits 36-47)
    // raw_glucose = ((input[4] & 0x0F) << 8) | input[5];

    // raw_glucose bits is byte 0 and the first 6 bits of byte 1 (bits 0-13) (additional 2 bits from previous implementation bits 0-11)
    raw_glucose = readBits(input, 0, 0, 0xE);

    // calculate glucose value
    glucose_value = calibration->calibrate(raw_glucose, raw_temperature, temperature_adjustment);
    if (glucose_value < 0) {
        glucose_value = 0;
    }
}

/*

(pre-reversal) BLE record layout:

gggg gggg gggg --tt
tttt tttt ttaa aaan

g = raw glucose
t = raw temperature
a = temperature adjustment
n = negative

(post-reversal) BLE record layout:

naaa aatt tttt tttt
tt-- gggg gggg gggg

*/

ble_record::ble_record(const std::shared_ptr<FactoryCalibration> calibration, const std::vector<uint8_t> record) {
    // check if BLE record is valid (4 bytes)
    if (record.size() != BLE_RECORD_SIZE_BYTES) {
        // TODO: error message / logging
        negative = (temperature_adjustment = (raw_temperature = (raw_glucose = 0)));
        return;
    }

    auto input = record;

    // reverse the bytes in the record
    // std::reverse(input.begin(), input.end());

    // reverse the bits in each byte
    // for (auto& byte : input) {
    //     byte = reverse_bits(byte);
    // }

    // negative bit is now the first bit (bit 0)
    // negative = (input[0] >> 7) & 0x01;

    // negative bit is the final bit of the final byte (bit 31)
    negative = readBits(input, 0, 0x1F, 0x1);

    // temperature_adjustment bits are bits 1-5 of byte 0
    // temperature_adjustment = input[0] & 0x7C;

    // temperature_adjustment bits are bits 2-6 of the final byte (bits 26-30)
    temperature_adjustment = readBits(input, 0, 0x1A, 0x5) << 2;
    if (negative != 0) {
        temperature_adjustment = -temperature_adjustment;
    }

    // raw_temperature bits are now the final 2 bits of byte 0, the entirety of byte 1, and the first 2 bits of byte 2 (bits 6-17)
    // raw_temperature = ((input[0] & 0x03) << 10) | (input[1] << 2) | (input[2] >> 6);

    // raw_temperature is the final 2 bits of byte 1, all of byte 2, and the first 2 bits of byte 3 (bits 14-25)
    raw_temperature = readBits(input, 0, 0xE, 0xC) << 2;

    // raw_glucose bits are now the final byte and the final 4 bits of byte 3 (bits 20-31)
    // raw_glucose = ((input[3] & 0x0F) << 8) | input[4];

    // raw_glucose is byte 0 and the first 6 bits of byte 1 (bits 0-13) (additional 2 bits from previous implementation bits 0-11)
    raw_glucose = readBits(input, 0, 0, 0xE);
}

} // namespace cgm
