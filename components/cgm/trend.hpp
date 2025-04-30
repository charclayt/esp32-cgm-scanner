#ifndef __CGM_TREND_HPP__
#define __CGM_TREND_HPP__

#include <string>

namespace cgm {

/**
 * @brief glucose_trend enum to define glucose trends, to be used to display trend arrows
 * 
 */
enum class glucose_trend {
    STEADY,
    RISING_SLOW,
    RISING_FAST,
    FALLING_SLOW,
    FALLING_FAST,
};

/**
 * @brief Convert glucose_trend to string
 * 
 * @param trend the glucose trend to convert
 * @return `std::string` - the string representation of the glucose trend
 */
std::string to_string(glucose_trend trend);

} // namespace cgm

#endif // __CGM_TREND_HPP__
