#ifndef __CGM_RECORD_HPP__
#define __CGM_RECORD_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief 
 * 
 */
struct fram_record {

    /**
     * @brief default constructor for fram record
     * 
     */
    fram_record() = default;

    /**
     * @brief Construct a new fram record object
     * 
     * @param record 
     */
    fram_record(const std::vector<uint8_t> record);

    bool has_error;
    bool negative;

    int raw_glucose;
    int raw_temperature;
    int temperature_adjustment;
};

/**
 * @brief 
 * 
 */
struct ble_record {
    /**
     * @brief Construct a new ble record object
     * 
     */
    ble_record() = default;

    bool negative;

    std::vector<uint8_t> raw_glucose;
    std::vector<uint8_t> raw_temperature;
    std::vector<uint8_t> temperature_adjustment;
};

} // namespace cgm

#endif // __CGM_RECORD_HPP__
