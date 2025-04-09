#include <calculations.hpp>

#include <algorithm>
#include <record.hpp>
#include <trend.hpp>

#include <esp_log.h>

#define TAG "CGM_CALCULATIONS"

namespace cgm {

double calculate_glucose_mmol(fram_record& record) {
    double glucose_mmol = record.raw_glucose / 18; // Convert to mmol/L

    // TODO: apply calibration if needed (temperature adjustment, etc)

    return glucose_mmol;
}

glucose_trend calculate_glucose_roc(std::vector<fram_record> glucose_vector) {
    if (glucose_vector.size() < 2) {
        return glucose_trend::STEADY; // Not enough data to determine trend
    }

    ESP_LOGD(TAG, "glucose_vector size: %d", glucose_vector.size());

    // reverse the vector to get oldest to newest values
    std::reverse(glucose_vector.begin(), glucose_vector.end());

    // TODO: maybe perform calibration on glucose_vector first

    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

    for (auto i = 0; i < glucose_vector.size(); i++) {
        double x = i;
        double y = glucose_vector[i].raw_glucose;

        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    double numerator = glucose_vector.size() * sumXY - sumX * sumY;
    double denominator = glucose_vector.size() * sumX2 - sumX * sumX;

    if (denominator == 0) {
        return glucose_trend::STEADY; // Avoid division by zero
    }

    auto slope = numerator / denominator;
    ESP_LOGD(TAG, "Glucose ROC slope: %f", slope);

    if (slope >= 2.0) {
        return glucose_trend::RISING_FAST;
    } else if (slope >= 1.0) {
        return glucose_trend::RISING_SLOW;
    } else if (slope <= -1.0) {
        return glucose_trend::FALLING_SLOW;
    } else if (slope <= -2.0) {
        return glucose_trend::FALLING_FAST;
    } else {
        return glucose_trend::STEADY;
    }
}

} // namespace cgm
