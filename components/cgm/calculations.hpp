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
 * @param glucose_vector the fram_record vector containing the glucose values from the past 16 minutes
 * @return `glucose_trend` - the glucose trend based on the rate of change
 */
glucose_trend calculate_glucose_roc(std::vector<fram_record> glucose_vector);

/**
 * @overload calculate_glucose_roc(std::vector<ble_record> glucose_vector)
 */
glucose_trend calculate_glucose_roc(std::vector<ble_record> glucose_vector);

} // namespace cgm

#endif // __CGM__CALCULATIONS_HPP__
