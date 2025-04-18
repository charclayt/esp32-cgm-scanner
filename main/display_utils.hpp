#ifndef __DISPLAY_HPP__
#define __DISPLAY_HPP__

#include <fram.hpp>
#include <trend.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/**
 * @brief draw the default display
 * 
 * @param display the display object
 */
void draw_default_display(Adafruit_SSD1306& display);

/**
 * @brief draw the real-time glucose display, including glucose level, time since last reading, trend, and 15 minute prediction
 * 
 * @param display the display object
 * @param glucose_level the current glucose level
 * @param time_since_reading time since last reading without error
 * @param trend the glucose trend
 * @param predicted_glucose the predicted glucose level in 15 minutes
 */
void draw_glucose_display(Adafruit_SSD1306& display, double glucose_level, int time_since_reading, cgm::glucose_trend trend, double predicted_glucose);

/**
 * @brief draw the historic glucose display, including a graph of the last 8 hours of glucose data, and the average glucose level in the timeframe
 * 
 * @param display the display object
 * @param current_glucose the current glucose level
 * @param historic_records vector of historic records
 */
void draw_historic_display(Adafruit_SSD1306& display, double current_glucose, std::vector<cgm::fram_record>& historic_records);

/**
 * @brief draw the error display, including the given error message
 * 
 * @param display the display object
 * @param error_message the custom error message to display
 */
void display_error(Adafruit_SSD1306& display, std::optional<std::string> error_message);

#endif // __DISPLAY_HPP__
