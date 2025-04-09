#ifndef __CGM_FRAM_HPP__
#define __CGM_FRAM_HPP__

#include <record.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

/**
 * @brief FRAM_data struct to hold the FRAM data in its individual components
 * 
 */
struct FRAM_data {

    /**
     * @brief default constructor for FRAM_data
     * 
     */
    FRAM_data() = default;

    /**
     * @brief Construct a new fram data object
     * 
     * @param fram the FRAM data to be parsed
     */
    FRAM_data(const std::vector<uint8_t>& fram);

    int error = false;

    // FRAM data components:
    
    /// HEADER ///
    std::vector<uint8_t> header_crc; // byte 0-1
    uint8_t sensor_state; // byte 4
    
    /// BODY ///
    std::vector<uint8_t> body_crc; // byte 24-25
    uint8_t trend_index; // byte 26
    uint8_t historic_index; // byte 27


    // 16 trend records (6 bytes each) are the values read for the last 16 minutes before it is saved as a historic record
    // a trend record is overwritten each minute, and the current record is the one that the trend_index (pointer) points to

    // ----------------------------------------------------- trend record -----------------------------------------------------
    // |  1   |  2   |  3   |  4   |  5   |  6   |  7   |  8   |  9   | 10  |  11  |  12  |   13    |  14   |   15   |   16   |
    // (28-33, 34-39, 40-45, 46-51, 52-57, 58-63, 64-69, 70-75, 76-81, 82-87, 88-93, 94-99, 100-105, 106-111, 112-117, 118-123)
    std::vector<fram_record> trend_records; // bytes 28-123


    // 32 historic records (6 bytes each) are computed every 15 minutes for the last 8 hours
    // a historic record is computed from the trend values by averaging the values obtained in the trend

    // --------------------------------------------------------------- historic records ---------------------------------------------------------------
    // |   1    |   2    |   3    |   4    |   5    |   6    |   7    |   8    |   9    |   10   |   11   |   12   |   13   |  14   |   15   |   16   |
    // (124-129, 130-135, 136-141, 142-147, 148-153, 154-159, 160-165, 166-171, 172-177, 178-183, 184-189, 190-195, 196-201, 202-207, 208-213, 214-219)
    // |   17   |   18   |   19   |   20   |   21   |   22   |   23   |   24   |   25   |   26   |   27   |   28   |   29   |   30   |   31   |   32  |
    // (220-225, 226-231, 232-237, 238-243, 244-249, 250-255, 256-261, 262-267, 268-273, 274-279, 280-285, 286-291, 292-297, 298-303, 304-309, 310-315)
    std::vector<fram_record> historic_records; // bytes 124-315


    std::vector<uint8_t> age; // byte 316-317

    /// FOOTER ///
    std::vector<uint8_t> footer_crc; // byte 320-321
    std::vector<uint8_t> max_life; // byte 326-327
};

} // namespace cgm

#endif // __CGM_FRAM_HPP__
