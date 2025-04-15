#ifndef __CGM_NFC_HPP__
#define __CGM_NFC_HPP__

#include <vector>
#include <cstdint>

#include "PN5180ISO15693.h"

namespace cgm {

class CGM_NFC : public PN5180ISO15693 {

public:

using PN5180ISO15693::PN5180ISO15693;

/**
 * @brief 
 * 
 * @param uid 
 * @param block_size 
 * @param buffer 
 */
void read_FRAM(std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer);

/**
 * @brief 
 * 
 * @param nfc
 * @param uid
 * @param block_size
 * @param buffer
 * 
 * @return populates the buffer with the header data
 */
void readHeader(PN5180ISO15693& nfc, std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer);

/**
 * @brief 
 * 
 * @param nfc
 * @param uid
 * @param block_size
 * @param buffer
 * 
 * @return populates the buffer with the body data
 */
void readBody(const PN5180ISO15693& nfc, const std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer);

/**
 * @brief 
 * 
 * @param nfc
 * @param uid
 * @param block_size
 * @param buffer
 * 
 * @return populates the buffer with the footer data
 */
void readFooter(const PN5180ISO15693& nfc, const std::vector<uint8_t>& uid, const uint8_t block_size, std::vector<uint8_t>& buffer);

};

} // namespace cgm

#endif // __CGM_NFC_HPP__
