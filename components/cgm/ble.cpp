#include <ble.hpp>

#include <record.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

    BLE_data::BLE_data(std::vector<uint8_t>& ble) {
        trend_records.reserve(sizeof(ble_record) * 7); // 7 trend records (4 bytes each) for the last 7 minutes
        trend_records.push_back(ble_record({ble[0], ble[1], ble[2], ble[3]}));
        trend_records.push_back(ble_record({ble[4], ble[5], ble[6], ble[7]}));
        trend_records.push_back(ble_record({ble[8], ble[9], ble[10], ble[11]}));
        trend_records.push_back(ble_record({ble[12], ble[13], ble[14], ble[15]}));
        trend_records.push_back(ble_record({ble[16], ble[17], ble[18], ble[19]}));
        trend_records.push_back(ble_record({ble[20], ble[21], ble[22], ble[23]}));
        trend_records.push_back(ble_record({ble[24], ble[25], ble[26], ble[27]}));

        historic_records.reserve(sizeof(ble_record) * 3); // 3 historic records (4 bytes each) for the last 3 15-minute averages
        historic_records.push_back(ble_record({ble[28], ble[29], ble[30], ble[31]}));
        historic_records.push_back(ble_record({ble[32], ble[33], ble[34], ble[35]}));
        historic_records.push_back(ble_record({ble[36], ble[37], ble[38], ble[39]}));

        age = {ble[40], ble[41]};

        ble_crc = {ble[42], ble[43]};
    }

} // namespace cgm
