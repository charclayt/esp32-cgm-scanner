#ifndef __CGM__CALCULATIONS_HPP__
#define __CGM__CALCULATIONS_HPP__

#include <record.hpp>
#include <trend.hpp>

#include <vector>
#include <cstdint>

namespace cgm {

double calculate_glucose_mmol(fram_record& record);

double calculate_glucose_mmol(ble_record& record);

/**
 * @brief calculate the rate of change of glucose values given using linear regression, for use in trend arrows, etc
 * 
 * @param glucose_vector the fram_record vector containing the glucose values from the past `n` minutes (with errors removed if needed)
 * @return `glucose_trend` - the glucose trend based on the rate of change
 * 
 * @note the glucose_vector should be in contiguous order, so that the first element is the most recent record, and the last element is the oldest record.
 * @note consider calling `calculate_contiguous_records()` first to get the records in contiguous order, if unsorted
 */
glucose_trend calculate_glucose_roc(std::vector<fram_record> glucose_vector);

/**
 * @overload calculate_glucose_roc(std::vector<ble_record> glucose_vector)
 */
glucose_trend calculate_glucose_roc(std::vector<ble_record> glucose_vector);

/// Helper functions ///

/**
 * @brief FRAM records are rotating, so not always contiguous, this function will output records in a contiguous order
 * 
 * @note the current trend record is pointed to by the trend_index in FRAM, and the previous records are in reverse order. If trend_index != records.size(), then once the iterator reaches 0, it will loop back to the end of the vector and continue until it reaches trend_index + 1
 * 
 * @param records the records to be ordered
 * @param start_index the index pointed to by the trend_index indicating current position
 * @param remove_errors whether to remove error records or not
 * @return `std::vector<fram_record>` - the records in contiguous order
 */
std::vector<fram_record> calculate_contiguous_records(std::vector<fram_record>& records, int start_index, bool remove_errors);

} // namespace cgm

#endif // __CGM__CALCULATIONS_HPP__
