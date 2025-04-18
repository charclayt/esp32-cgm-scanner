#include <sensor/factory_calibration.hpp>

#include <common.hpp>

#include <cmath>
#include <cstdint>
#include <vector>

#include <esp_log.h>
#define TAG "CGM_FACTORY_CALIBRATION"

namespace cgm {

double FactoryCalibration::calibrate(double raw_glucose, double raw_temperature, double raw_temperature_adjustment) const {
    if (raw_glucose <= 0 || i2 == 0) {
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

    double R = (r_left / r_right) - y;

    double log_R = std::log(R);
    double d = std::pow(log_R, 3) * cd + std::pow(log_R, 2) * cc + log_R * cb + ca;

    double temperature = 1 / d - 273.15;

    double g1 = 65.0 * (raw_glucose - i3) / (i4 - i3);
    double g2 = std::pow(1.045, 32.5 - temperature);

    double g3 = g1 * g2;

    double v1 = t1[i2 - 1];
    double v2 = t2[i2 - 1];

    auto value = std::round((g3 - v1) / v2);
    ESP_LOGD(TAG, "raw_glucose: %f, raw_temperature: %f, raw_temperature_adjustment: %f, calibrated_glucose: %f", raw_glucose, raw_temperature, raw_temperature_adjustment, value);

    return value;
}

FactoryCalibration FactoryCalibration::libre_calibration(const std::vector<uint8_t>& fram) {
    int i1 = readBits(fram, 2, 0, 3);
    int i2 = readBits(fram, 2, 3, 0xa);

    double i3 = static_cast<double>(readBits(fram, 0x150, 0, 8));
    if (readBits(fram, 0x150, 0x21, 1) != 0) {
        i3 = -i3;
    }

    double i4 = static_cast<double>(readBits(fram, 0x150, 8, 0xe));
    double i5 = static_cast<double>(readBits(fram, 0x150, 0x28, 0xc) << 2);
    double i6 = static_cast<double>(readBits(fram, 0x150, 0x34, 0xc) << 2);

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
