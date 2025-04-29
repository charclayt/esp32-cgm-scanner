#include <sensor/factory_calibration.hpp>

#include <common.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

#include <esp_log.h>
#define TAG "CGM_FACTORY_CALIBRATION"

namespace cgm {

int FactoryCalibration::calibrate(double raw_glucose, double raw_temperature, double raw_temperature_adjustment) const {
    if (raw_glucose <= 0 || i2 == 0 || i3 == 0 || i4 == 0 || i6 == 0) {
        ESP_LOGE(TAG, "Invalid input - raw_glucose: %f, i2: %d, i3: %f, i4: %f, i6: %f", raw_glucose, i2, i3, i4, i6);
        return raw_glucose;
    }

    double x = 1000 + 71500;
    double y = 1000;
    double ca = 0.0009180023;
    double cb = 0.0001964561;
    double cc = 0.0000007061775;
    double cd = 0.00000005283566;

    double r_left = raw_temperature * x;
    double r_right = raw_temperature_adjustment + i6;

    if (r_right == 0) {
        ESP_LOGE(TAG, "Division by zero - raw_temperature_adjustment: %f, i6: %f", raw_temperature_adjustment, i6);
        return -1; // Or handle gracefully
    }

    double R = (r_left / r_right) - y;

    if (R <= 0) {
        ESP_LOGE(TAG, "Invalid R value: %f. raw_temperature: %f, raw_temperature_adjustment: %f, i6: %f", R, raw_temperature, raw_temperature_adjustment, i6);
        return -1; // Or handle gracefully
    }

    double log_R = std::log(R);
    double d = std::pow(log_R, 3) * cd + std::pow(log_R, 2) * cc + log_R * cb + ca;

    double temperature = 1 / d - 273.15;

    // Validate i2 index
    if (i2 - 1 < 0 || i2 - 1 >= sizeof(t1)) {
        ESP_LOGE(TAG, "Invalid i2 index: %d. Array bounds exceeded.", i2);
        return -1;
    }

    double g1 = 65.0 * (raw_glucose - i3) / (i4 - i3);
    double g2 = std::pow(1.045, 32.5 - temperature);
    double g3 = g1 * g2;

    double v1 = t1[i2 - 1];
    double v2 = t2[i2 - 1];

    int value = std::round((g3 - v1) / v2);

    if (value < 0) {
        ESP_LOGE(TAG, "Calibrated glucose is negative: %d. Clamping to zero.", value);
        value = 0;
    }

    ESP_LOGD(TAG, "raw_glucose: %f, raw_temperature: %f, raw_temperature_adjustment: %f, calibrated_glucose: %d, calibrated_temperature: %f",
             raw_glucose, raw_temperature, raw_temperature_adjustment, value, temperature);

    ESP_LOGD(TAG, "Calibration params - i3: %f, i4: %f, i6: %f, i2: %d, t1[i2-1]: %f, t2[i2-1]: %f",
         i3, i4, i6, i2, t1[i2-1], t2[i2-1]);

    // TODO: Calibration currently seems to be off by a factor of 3, so we divide by 3 as a temporary fix
    return value / 2;
}

FactoryCalibration FactoryCalibration::libre_calibration(const std::vector<uint8_t>& fram) {
    int i1 = read_bits(fram, 2, 0, 3);
    int i2 = read_bits(fram, 2, 3, 0xa);

    double i3 = static_cast<double>(read_bits(fram, 0x150, 0, 8));
    if (read_bits(fram, 0x150, 0x21, 1) != 0) {
        i3 = -i3;
    }

    double i4 = static_cast<double>(read_bits(fram, 0x150, 8, 0xe));
    double i5 = static_cast<double>(read_bits(fram, 0x150, 0x28, 0xc) << 2);
    double i6 = static_cast<double>(read_bits(fram, 0x150, 0x34, 0xc) << 2);

    return FactoryCalibration(i1, i2, i3, i4, i5, i6);

    // int i1 = fram[2] & 0x07; // last 3 bits of byte 2
    // int i2 = (fram[2] >> 3) & 0x1F; // next 5 bits of byte 2

    // double i3 = static_cast<double>(fram[0x150]);
    // // check if the sign bit is set
    // if ((fram[(0x150 + 4) & 0x01]) != 0) {
    //     i3 = -i3;
    // }

    // double i4 = (static_cast<double>(fram[0x150 + 1]) * 256.0) + fram[0x150 + 2];
    // double i5 = ((static_cast<double>(fram[0x150 + 5]) * 256.0) + fram[0x150 + 6]) * 4.0;
    // double i6 = ((static_cast<double>(fram[0x150 + 7]) * 256.0) + fram[0x150 + 8]) * 4.0;

    // return FactoryCalibration(i1, i2, i3, i4, i5, i6);
}

} // namespace cgm
