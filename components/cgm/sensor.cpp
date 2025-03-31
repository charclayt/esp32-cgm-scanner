#include <sensor.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

void sensor::set_UID(const uint8_t* uid) {
    m_uid.reserve(sizeof(uid));
    m_uid = std::vector<uint8_t>(uid, uid + sizeof(uid));
}

std::vector<uint8_t> sensor::get_UID() {
    return m_uid;
}

const uint8_t* sensor::get_array_UID() const {
    return m_uid.data();
}

} // namespace cgm
