#ifndef __CGM_BLE_HPP__
#define __CGM_BLE_HPP__

#include <record.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief BLE_data struct to hold the BLE data in its indivudal components
 * 
 */
struct BLE_data {

    /**
     * @brief Construct a new ble data object
     * 
     * @param ble the BLE data to be parsed
     */
    BLE_data(std::vector<uint8_t>& ble);

    // BLE data components:


    // 10 BLE records (4 bytes each = 40 bytes) - the first 7 blocks correspond to trend values and the last 3 to historic values
    // the trend values correspond to blocks t, t2, t4, t6, t7, t12, and t15 where t = the minute at which the values are sent
    // the historic values correspond to blocks h1, h2, and h3 which are the most recent consecutive 15 minute averages

    // ------------------- trend records -------------------
    // |  t  |  t2  |  t4  |   t6  |   t7  |  t12  |  t15  |
    // -----------------------------------------------------
    // | 0-3 |  4-7 | 8-11 | 12-15 | 16-19 | 20-23 | 24-27 |

    // ---- historic records ---
    // |  h1   |  h2   |  h3   |
    // -------------------------
    // | 28-31 | 32-35 | 36-39 |

    std::vector<ble_record> records; // bytes 0-39

    std::vector<uint8_t> age; // byte 40-41

    std::vector<uint8_t> ble_crc; // byte 42-43
};

} // namespace cgm

#endif // __CGM_BLE_HPP__
