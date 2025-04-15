#ifndef __CGM_SENSOR_HPP__
#define __CGM_SENSOR_HPP__

#include <fram.hpp>
#include <ble.hpp>
#include <sensor/sensor_family.hpp>
#include <sensor/sensor_region.hpp>
#include <sensor/sensor_serial.hpp>
#include <sensor/sensor_state.hpp>
#include <sensor/sensor_types.hpp>

#include <vector>
#include <cstdint>
#include <string>

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
};

} // namespace cgm

#endif // __CGM_SENSOR_HPP__
