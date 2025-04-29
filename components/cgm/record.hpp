#ifndef __CGM_RECORD_HPP__
#define __CGM_RECORD_HPP__

#include <sensor/factory_calibration.hpp>

#include <memory>
#include <cstdint>
#include <vector>

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
     * @param calibration the factory calibration to be used
     * @param record the record to be parsed
     */
    fram_record(const std::shared_ptr<FactoryCalibration> calibration, std::vector<uint8_t> record);


    bool negative;
    bool has_error;
    double raw_glucose;
    double raw_temperature;
    double temperature_adjustment;

    int glucose_value;
    int offset;
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
     * @param calibration the factory calibration to be used
     * @param record the record to be parsed
     */
    ble_record(const std::shared_ptr<FactoryCalibration> calibration, const std::vector<uint8_t> record);


    bool negative;
    bool has_error = false;
    double raw_glucose;
    double raw_temperature;
    double temperature_adjustment;

    double glucose_value;
};

} // namespace cgm

#endif // __CGM_RECORD_HPP__
