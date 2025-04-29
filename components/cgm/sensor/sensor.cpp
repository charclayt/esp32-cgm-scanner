#include <sensor/sensor.hpp>

#include <decrypt.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

namespace cgm {

void sensor::initialise(uint8_t* uid, uint8_t* patch_info, std::vector<uint8_t> fram, std::shared_ptr<FactoryCalibration> calibration) {
    m_calibration = std::move(calibration);
    initialise(uid, patch_info, fram);
}

void sensor::initialise(uint8_t* uid, uint8_t* patch_info, std::vector<uint8_t> fram) {
    m_uid.assign(uid, uid + 8);
    m_patch_info.assign(patch_info, patch_info + 6);

    auto decrypted_fram = decrypt_FRAM(m_uid, m_patch_info, fram);

    if (!m_calibration) {
        m_calibration = std::make_shared<FactoryCalibration>(FactoryCalibration::libre_calibration(decrypted_fram));
    }

    m_fram_data = FRAM_data(m_calibration, decrypted_fram);
    // TODO: error handling
    // if (m_fram_data.error)
    // Assign timestamps to trend records
    // auto now = std::chrono::system_clock::now();
    // auto time_in_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    // int buffer_size = m_fram_data.trend_records.size();

    // for (int i = 0; i < buffer_size; ++i) {
    //     // int index = (m_fram_data.trend_index - i + buffer_size) % buffer_size;
    //     // m_fram_data.trend_records[index].timestamp = time_in_seconds - (i * 60); // Each record is 1 min older
    //     int index = (m_fram_data.trend_index - i + buffer_size) % buffer_size;  // Wrap around the buffer correctly
    //     int time_offset = (buffer_size - i) * 60;  // Calculate the correct minute offset for this record

    //     // Assign timestamp
    //     m_fram_data.trend_records[index].timestamp = time_in_seconds - time_offset;
    // }
    // for (int i = m_fram_data.trend_index; i >= 0; --i) {
    //     m_fram_data.trend_records[i].timestamp = time_in_seconds - ((m_fram_data.trend_index - i) * 60); // Each record is 1 min older
    // }
    // for (int i = m_fram_data.trend_records.size() - 1; i > m_fram_data.trend_index; --i) {
    //     m_fram_data.trend_records[i].timestamp = time_in_seconds - ((m_fram_data.trend_index + (m_fram_data.trend_records.size() - i)) * 60); // Each record is 1 min older
    // }

    m_serial_numer = get_sensor_serial_number(m_uid, m_patch_info);
    m_family = get_sensor_family(m_patch_info);
    m_region = get_sensor_region(m_patch_info);
    m_state = get_sensor_state(m_fram_data.sensor_state);
    m_type = get_sensor_type(m_patch_info);
}

void sensor::set_UID(const uint8_t* uid) {
    // m_uid.reserve(sizeof(uid));
    // m_uid = std::vector<uint8_t>(uid, uid + sizeof(uid));
    this->m_uid.assign(uid, uid + 8); // Ensure proper copying
}

std::vector<uint8_t> sensor::get_UID() {
    return m_uid;
}

const uint8_t* sensor::get_array_UID() const {
    return m_uid.data();
}

} // namespace cgm
