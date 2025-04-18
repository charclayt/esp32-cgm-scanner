#ifndef __CGM_COMMON_HPP__
#define __CGM_COMMON_HPP__

#include <cstdint>
#include <vector>

namespace cgm {

const auto FRAM_SIZE_BLOCKS = 43;
const auto FRAM_RECORD_SIZE_BYTES = 6;

const auto NUM_FRAM_TREND_RECORDS = 16;
const auto NUM_FRAM_HISTORIC_RECORDS = 32;

const auto BLE_SIZE_BYTES = 43;
const auto BLE_RECORD_SIZE_BYTES = 4;

int readBits(const std::vector<uint8_t>& buffer, int byte_offset, int bit_offset, int bit_count);

} // namespace cgm

#endif // __CGM_COMMON_HPP__
