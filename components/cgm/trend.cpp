#include <trend.hpp>

#include <string>

namespace cgm {

std::string to_string(glucose_trend trend) {
    switch (trend) {
        case glucose_trend::STEADY:
            return "Steady";
        case glucose_trend::RISING_SLOW:
            return "Rising Slow";
        case glucose_trend::RISING_FAST:
            return "Rising Fast";
        case glucose_trend::FALLING_SLOW:
            return "Falling Slow";
        case glucose_trend::FALLING_FAST:
            return "Falling Fast";
        default:
            return "Unknown";
    }
}

} // namespace cgm
