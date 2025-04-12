#ifndef __DISPLAY_HPP__
#define __DISPLAY_HPP__

#include <trend.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void draw_default_display(Adafruit_SSD1306& display);

void draw_glucose_display(Adafruit_SSD1306& display, double glucose_level, int time_since_reading, cgm::glucose_trend trend = cgm::glucose_trend::STEADY);

void display_error(Adafruit_SSD1306& display, std::optional<std::string> error_message);

#endif // __DISPLAY_HPP__
