#ifndef __CGM_FRAM_HPP__
#define __CGM_FRAM_HPP__

#include <record.hpp>

#include <cstdint>
#include <vector>

namespace cgm {

/**
 * @brief FRAM_data struct to hold the FRAM data in its indivudal components
 * 
 */
struct FRAM_data {

    /**
     * @brief Construct a new fram data object
     * 
     * @param fram 
     */
    FRAM_data(std::vector<uint8_t>& fram);

    // FRAM data components
    
    /// HEADER ///
    std::vector<uint8_t> header_crc; // byte 0-1
    uint8_t sensor_state; // byte 4
    
    /// BODY ///
    std::vector<uint8_t> body_crc; // byte 24-25
    uint8_t trend_index; // byte 26
    uint8_t historic_index; // byte 27

    // 16 trend records (6 bytes each) are the values read for the last 16 minutes before it is saved as a historic record
    // a trend record is overwritten each minute, and the current record is the one that the trend_index (pointer) points to
    fram_record trend_record_1; // byte 28-33
    fram_record trend_record_2; // byte 34-39
    fram_record trend_record_3; // byte 40-45
    fram_record trend_record_4; // byte 46-51
    fram_record trend_record_5; // byte 52-57
    fram_record trend_record_6; // byte 58-63
    fram_record trend_record_7; // byte 64-69
    fram_record trend_record_8; // byte 70-75
    fram_record trend_record_9; // byte 76-81
    fram_record trend_record_10; // byte 82-87
    fram_record trend_record_11; // byte 88-93
    fram_record trend_record_12; // byte 94-99
    fram_record trend_record_13; // byte 100-105
    fram_record trend_record_14; // byte 106-111
    fram_record trend_record_15; // byte 112-117
    fram_record trend_record_16; // byte 118-123
    std::vector<fram_record> trend_records;

    // 32 historic records (6 bytes each) are computed every 15 minutes for the last 8 hours
    // a historic record is computed from the trend values by averaging the values obtained in the trend
    fram_record historic_record_1; // byte 124-129
    fram_record historic_record_2; // byte 130-135
    fram_record historic_record_3; // byte 136-141
    fram_record historic_record_4; // byte 142-147
    fram_record historic_record_5; // byte 148-153
    fram_record historic_record_6; // byte 154-159
    fram_record historic_record_7; // byte 160-165
    fram_record historic_record_8; // byte 166-171
    fram_record historic_record_9; // byte 172-177
    fram_record historic_record_10; // byte 178-183
    fram_record historic_record_11; // byte 184-189
    fram_record historic_record_12; // byte 190-195
    fram_record historic_record_13; // byte 196-201
    fram_record historic_record_14; // byte 202-207
    fram_record historic_record_15; // byte 208-213
    fram_record historic_record_16; // byte 214-219
    fram_record historic_record_17; // byte 220-225
    fram_record historic_record_18; // byte 226-231
    fram_record historic_record_19; // byte 232-237
    fram_record historic_record_20; // byte 238-243
    fram_record historic_record_21; // byte 244-249
    fram_record historic_record_22; // byte 250-255
    fram_record historic_record_23; // byte 256-261
    fram_record historic_record_24; // byte 262-267
    fram_record historic_record_25; // byte 268-273
    fram_record historic_record_26; // byte 274-279
    fram_record historic_record_27; // byte 280-285
    fram_record historic_record_28; // byte 286-291
    fram_record historic_record_29; // byte 292-297
    fram_record historic_record_30; // byte 298-303
    fram_record historic_record_31; // byte 304-309
    fram_record historic_record_32; // byte 310-315

    std::vector<uint8_t> age; // byte 316-317

    /// FOOTER ///
    std::vector<uint8_t> footer_crc; // byte 320-321
    std::vector<uint8_t> max_life; // byte 326-327
};

} // namespace cgm

#endif // __CGM_FRAM_HPP__
