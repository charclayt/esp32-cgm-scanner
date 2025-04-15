#ifndef __CGM_RECORD_HPP__
#define __CGM_RECORD_HPP__

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief a record read from the sensor FRAM containing glucose and temperature data
 * 
 */
struct fram_record {

    /**
     * @brief Default constructor for fram record
     * 
     */
    fram_record() = default;

    /**
     * @brief Construct a new fram record object
     * 
     * @param record the record to be parsed
     */
    fram_record(const std::vector<uint8_t> record);


    bool negative;
    bool has_error;
    int raw_glucose;
    int raw_temperature;
    int temperature_adjustment;
};

/**
 * @brief a record received from the sensor via BLE containing glucose and temperature data
 * 
 */
struct ble_record {
    /**
     * @brief Default constructor for ble record
     * 
     */
    ble_record() = default;

    /**
     * @brief Construct a new ble record object
     * 
     * @param record the record to be parsed
     */
    ble_record(const std::vector<uint8_t> record);


    bool negative;
    int raw_glucose;
    int raw_temperature;
    int temperature_adjustment;
};

} // namespace cgm

#endif // __CGM_RECORD_HPP__
