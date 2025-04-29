#ifndef __CGM_SENSOR_HPP__
#define __CGM_SENSOR_HPP__

#include <ble.hpp>
#include <fram.hpp>
#include <sensor/factory_calibration.hpp>
#include <sensor/sensor_family.hpp>
#include <sensor/sensor_region.hpp>
#include <sensor/sensor_serial.hpp>
#include <sensor/sensor_state.hpp>
#include <sensor/sensor_types.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace cgm {

/**
 * @brief 
 * 
 */
class sensor {

public:

    /**
     * @brief Construct a new Sensor object
     * 
     */
    sensor() = default;

    /**
     * @brief Initialise the sensor object
     * 
     * @param uid 
     * @param patch_info 
     * @param fram 
     */
    void initialise(uint8_t* uid, uint8_t* patch_info, std::vector<uint8_t> fram);

    /**
     * @brief Initialise the sensor object
     * 
     * @param uid 
     * @param patch_info 
     * @param fram 
     * @param calibration 
     */
    void initialise(uint8_t* uid, uint8_t* patch_info, std::vector<uint8_t> fram, std::shared_ptr<FactoryCalibration> calibration);

    /**
     * @brief Destroy the Sensor object
     * 
     */
    ~sensor() = default;

    /**
     * @brief Get the sensor UID object
     * 
     * @return `std::vector<uint8_t>` the sensor UID
     */
    std::vector<uint8_t> get_UID();

    /**
     * @brief Get the sensor UID object as an array
     * 
     * @return `const uint8_t*` the sensor UID
     */
    const uint8_t* get_array_UID() const;

    /**
     * @brief Set the sensor UID object
     * 
     * @param uid the sensor UID
     */
    void set_UID(const uint8_t* uid);

    /**
     * @brief Set the patch info object
     * 
     * @param patch_info the patch info
     */
    void set_patch_info(const uint8_t* patch_info);

// private:
    std::vector<uint8_t> m_uid;
    std::vector<uint8_t> m_patch_info;
    std::string m_serial_numer;
    sensor_family m_family;
    sensor_region m_region;
    sensor_state m_state;
    sensor_type m_type;

    FRAM_data m_fram_data;
    BLE_data m_ble_data;

    std::shared_ptr<FactoryCalibration> m_calibration;
};

} // namespace cgm

#endif // __CGM_SENSOR_HPP__
