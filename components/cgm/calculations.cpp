#include <calculations.hpp>

#include <algorithm>
#include <record.hpp>
#include <trend.hpp>

#include <esp_log.h>

#define TAG "CGM_CALCULATIONS"

namespace cgm {

double calculate_glucose_mmol(double raw_glucose) {
    // return raw_glucose * (1.0/180.182);
    return raw_glucose / 18.0;
    // return (raw_glucose * 10) * (1.0/180.182); // Convert to mmol/L
}

glucose_trend calculate_glucose_roc(std::vector<fram_record> glucose_vector) {
    if (glucose_vector.size() < 2) {
        return glucose_trend::STEADY; // Not enough data to determine trend
    }

    ESP_LOGD(TAG, "glucose_vector size: %d", glucose_vector.size());

    // Initialise the sums for linear regression
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

    // Loop through each record in the glucose_vector
    for (auto i = 0; i < glucose_vector.size(); i++) {
        double x = glucose_vector[i].offset; // Using offset instead of time (index difference)
        double y = glucose_vector[i].glucose_value;

        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
    }

    // Calculate the slope of the line using the linear regression formula
    double numerator = glucose_vector.size() * sumXY - sumX * sumY;
    double denominator = glucose_vector.size() * sumX2 - sumX * sumX;

    if (denominator == 0) {
        return glucose_trend::STEADY; // Avoid division by zero (vertical line)
    }

    // Calculate the slope
    auto slope = numerator / denominator;
    ESP_LOGD(TAG, "Glucose ROC slope: %f", slope);

    // Determine the trend based on the slope
    if (slope >= 2.0) {
        return glucose_trend::RISING_FAST;
    } else if (slope >= 1.0) {
        return glucose_trend::RISING_SLOW;
    } else if (-2.0 < slope && slope <= -1.0) {
        return glucose_trend::FALLING_SLOW;
    } else if (slope <= -2.0) {
        return glucose_trend::FALLING_FAST;
    } else {
        return glucose_trend::STEADY; // No significant change
    }
}

// glucose_trend calculate_glucose_roc(std::vector<fram_record> glucose_vector) {
//     if (glucose_vector.size() < 2) {
//         return glucose_trend::STEADY; // Not enough data to determine trend
//     }

//     ESP_LOGD(TAG, "glucose_vector size: %d", glucose_vector.size());

//     double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

//     for (auto i = 0; i < glucose_vector.size(); i++) {
//         double x = i;
//         double y = glucose_vector[i].glucose_value;

//         sumX += x;
//         sumY += y;
//         sumXY += x * y;
//         sumX2 += x * x;
//     }

//     double numerator = glucose_vector.size() * sumXY - sumX * sumY;
//     double denominator = glucose_vector.size() * sumX2 - sumX * sumX;

//     if (denominator == 0) {
//         return glucose_trend::STEADY; // Avoid division by zero
//     }

//     auto slope = numerator / denominator;
//     ESP_LOGD(TAG, "Glucose ROC slope: %f", slope);

//     if (slope >= 2.0) {
//         return glucose_trend::RISING_FAST;
//     } else if (slope >= 1.0) {
//         return glucose_trend::RISING_SLOW;
//     } else if (slope > -2.0 && slope <= -1.0) {
//         return glucose_trend::FALLING_SLOW;
//     } else if (slope <= -2.0) {
//         return glucose_trend::FALLING_FAST;
//     } else {
//         return glucose_trend::STEADY;
//     }
// }

double calculate_glucose_15_minute_predicton(glucose_trend trend, fram_record& current_record) {
    switch (trend) {
        case glucose_trend::RISING_FAST:
            return current_record.glucose_value + 30.0; // 2 mg/dL per minute
        case glucose_trend::RISING_SLOW:
            return current_record.glucose_value + 15.0; // 1 mg/dL per minute
        case glucose_trend::FALLING_FAST:
            return current_record.glucose_value - 30.0; // 2 mg/dL per minute
        case glucose_trend::FALLING_SLOW:
            return current_record.glucose_value - 15.0; // 1 mg/dL per minute
        default:
            return current_record.glucose_value; // No change
    }
}

std::vector<fram_record> calculate_contiguous_records(std::vector<fram_record>& records, int start_index, bool remove_errors) {
    std::vector<fram_record> contiguous_records;
    contiguous_records.reserve(records.size()); // Reserve space for all records

    // Start from the current trend index and go backwards
    for (int i = start_index; i >= 0; --i) {
        if (remove_errors && (records[i].has_error || records[i].glucose_value <= 0)) {
            continue; // Skip error records
        }
        records[i].offset = start_index - i; // Set the offset based on the current index
        contiguous_records.push_back(records[i]);
    }

    // Now go backwards from the end of the vector to the start index
    for (int i = records.size() - 1; i > start_index; --i) {
        if (remove_errors && (records[i].has_error || records[i].glucose_value <= 0)) {
            continue; // Skip error records
        }
        records[i].offset = start_index - i + records.size(); // Set the offset based on the current index
        contiguous_records.push_back(records[i]);
    }

    return contiguous_records;
}

} // namespace cgm
