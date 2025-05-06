#include <display_utils.hpp>

#include <common_main.hpp>

#include <calculations.hpp>
#include <fram.hpp>
#include <trend.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdint.h>
#include <format>
#include <numeric>

#define TAG "CGM_DISPLAY"

const uint8_t PROGMEM arrow_up[] = {0x08, 0x1c, 0x2a, 0x49, 0x08, 0x08, 0x08, 0x08};
const uint8_t PROGMEM arrow_up_diagonal[] = {0x0f, 0x03, 0x05, 0x09, 0x10, 0x20, 0x40, 0x80};
const uint8_t PROGMEM arrow_steady[] = {0x00, 0x08, 0x04, 0x02, 0xff, 0x02, 0x04, 0x08, 0x00};
const uint8_t PROGMEM arrow_down_diagonal[] = {0x80, 0x40, 0x20, 0x10, 0x09, 0x05, 0x03, 0x0f};
const uint8_t PROGMEM arrow_down[] = {0x08, 0x08, 0x08, 0x08, 0x49, 0x2a, 0x1c, 0x08};

static const unsigned char PROGMEM cgm_logo[] =
{ 0b00000111, 0b11100000,
  0b00011100, 0b00111000,
  0b00110000, 0b00001100,
  0b01100000, 0b00000110,
  0b01000000, 0b00000010,
  0b11000001, 0b10000011,
  0b10000011, 0b11000001,
  0b10000111, 0b11100001,
  0b10000111, 0b11100001,
  0b10000011, 0b11000001,
  0b11000001, 0b10000011,
  0b01000000, 0b00000010,
  0b01100000, 0b00000110,
  0b00110000, 0b00001100,
  0b00011100, 0b00111000,
  0b00000111, 0b11100000};

static const unsigned char PROGMEM cgm_logo_connection[] =
{ 0b00001000, 0b00000000,
  0b00000100, 0b00000000,
  0b00100100, 0b00000000,
  0b00010010, 0b00000000,
  0b10010010, 0b00000000,
  0b10001001, 0b00000000,
  0b01001001, 0b00000000,
  0b01001001, 0b00000000,
  0b01001001, 0b00000000,
  0b01001001, 0b00000000,
  0b10001001, 0b00000000,
  0b10010010, 0b00000000,
  0b00010010, 0b00000000,
  0b00100100, 0b00000000,
  0b00000100, 0b00000000,
  0b00001000, 0b00000000};

static const unsigned char PROGMEM fifteen_minute_icon[] =
{ 0b00000111, 0b11100000,
  0b00011001, 0b11111000,
  0b00100001, 0b11111100,
  0b01010001, 0b01000110,
  0b01001001, 0b01011110,
  0b10000001, 0b01000111,
  0b10000001, 0b01110111,
  0b11110001, 0b01000111,
  0b10000001, 0b11111111,
  0b10000000, 0b00000001,
  0b10000000, 0b00000001,
  0b01001000, 0b00010010,
  0b01010001, 0b00001010,
  0b00100001, 0b00000100,
  0b00011001, 0b00011000,
  0b00000111, 0b11100000};

static const unsigned char PROGMEM average_txt[] =
{ 0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b00000000, 0b00000000,
  0b11110100, 0b01011110,
  0b10010100, 0b01010000,
  0b10010100, 0b01010000,
  0b11110100, 0b01010110,
  0b10010010, 0b10010010,
  0b10010001, 0b00011110,
  0b00000000, 0b00000000};

void draw_default_display(Adafruit_SSD1306& display) {
    int16_t x1, y1;
    uint16_t w, h;

    display.clearDisplay();
    display.cp437(true);
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(1);
    display.getTextBounds("esp32-cgm-scanner", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.3));
    display.println("esp32-cgm-scanner");
    // TODO: switch between "esp32-cgm-scanner" and "please scan sensor" in main()

    // draw logo components
    display.drawBitmap((SCREEN_WIDTH - 32) / 2, (SCREEN_HEIGHT * 0.6), cgm_logo, 16, 16, SSD1306_WHITE);
    display.drawBitmap((SCREEN_WIDTH + 16) / 2, (SCREEN_HEIGHT * 0.6), cgm_logo_connection, 16, 16, SSD1306_WHITE);

    // TODO: draw NFC / BLE logo

    display.display();
    return;
}

void draw_glucose_display(Adafruit_SSD1306& display, double glucose_level, int time_since_reading, cgm::glucose_trend trend, double predicted_glucose) {
    int16_t x1, y1;
    uint16_t w, h;

    display.clearDisplay();
    display.cp437(true);

    // Set glucose value on the left side
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    auto glucose_str = std::format("{:.1f}", glucose_level).c_str();
    display.getTextBounds(glucose_str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH * 0.15), (SCREEN_HEIGHT - h) / 2); // vertically centered
    display.println(glucose_str);

    // Display time since last accurate reading (no errors)
    if (time_since_reading != 0) {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor((SCREEN_WIDTH * 0.15), (SCREEN_HEIGHT * 0.75));
        display.println(std::format("{} min ago", time_since_reading).c_str());
    }

    // Draw separation line
    display.drawLine(SCREEN_WIDTH * 0.7, SCREEN_HEIGHT, SCREEN_WIDTH * 0.7, 0, SSD1306_WHITE);

    // Display predicted glucose level (15 minutes) in bottom-right corner
    display.setTextSize(1);
    auto predicted_str = std::format("{:.1f}", predicted_glucose).c_str();
    display.getTextBounds(predicted_str, 0, 0, &x1, &y1, &w, &h);
    int16_t predicted_x = (SCREEN_WIDTH * 0.7 + SCREEN_WIDTH) / 2 - w / 2;
    int16_t predicted_y = SCREEN_HEIGHT * 0.8;
    display.setCursor(predicted_x, predicted_y);
    display.println(predicted_str);

    display.drawBitmap(predicted_x, predicted_y - 20, fifteen_minute_icon, 16, 16, SSD1306_WHITE);

    int16_t trend_x = predicted_x;
    int16_t trend_y = SCREEN_HEIGHT * 0.2;

    // Set arrow icon in top-right corner
    switch(trend) {
        case cgm::glucose_trend::STEADY:
            display.drawBitmap(trend_x, trend_y, arrow_steady, 8, 8, SSD1306_WHITE);
            break;
        case cgm::glucose_trend::RISING_SLOW:
            display.drawBitmap(trend_x, trend_y, arrow_up_diagonal, 8, 8, SSD1306_WHITE);
            break;
        case cgm::glucose_trend::RISING_FAST:
            display.drawBitmap(trend_x, trend_y, arrow_up, 8, 8, SSD1306_WHITE);
            break;
        case cgm::glucose_trend::FALLING_SLOW:
            display.drawBitmap(trend_x, trend_y, arrow_down_diagonal, 8, 8, SSD1306_WHITE);
            break;
        case cgm::glucose_trend::FALLING_FAST:
            display.drawBitmap(trend_x, trend_y, arrow_down, 8, 8, SSD1306_WHITE);
            break;
    }

    display.display();
    return;
}

void draw_historic_display(Adafruit_SSD1306& display, double current_glucose, std::vector<cgm::fram_record>& historic_records) {
    display.clearDisplay();
    display.cp437(true);

    std::vector<int> test_records = {152, 148, 148, 140, 136, 130, 125, 120, 120, 120, 125, 130, 132, 134, 130, 126, 130, 135, 140, 145, 142, 136, 132, 126, 124, 124, 120, 116, 118, 120, 122, 125};

    if (historic_records.empty()) {
        ESP_LOGE(TAG, "No historic records");
        display_error(display, "No historic records");
    }

    // Reverse to show most recent on the right
    std::reverse(historic_records.begin(), historic_records.end());

    // Display glucose values (debug)
    for (size_t i = 0; i < historic_records.size(); i++) {
        if (!historic_records[i].has_error) {
            auto glucose = cgm::calculate_glucose_mmol(historic_records[i].glucose_value);
            ESP_LOGD(TAG, "Glucose[%d] = %.1f mmol/L", (int)i, glucose);
        }
    }

    // Display parameters
    const int graph_x = 5;
    const int graph_y = 5;
    const int graph_width = display.width() - 40;
    const int graph_height = display.height() - 12;

    const double min_glucose = 0.0;
    const double max_glucose = 21.0;
    const double y_scale = graph_height / (max_glucose - min_glucose);

    const int total_points = historic_records.size();
    const double x_scale = static_cast<double>(graph_width) / (total_points - 1);

    // Draw axes
    display.drawLine(graph_x, graph_y + graph_height, graph_x, graph_y, WHITE);
    display.drawLine(graph_x, graph_y + graph_height, graph_x + graph_width, graph_y + graph_height, WHITE);

    // Draw line graph
    int prev_x = -1, prev_y = -1;
    for (size_t i = 0; i < historic_records.size(); ++i) {
        // if (historic_records[i].has_error || historic_records[i].glucose_value <= 0) {
        //     prev_x = prev_y = -1; // Break line segment
        //     continue;
        // }

        // double glucose = cgm::calculate_glucose_mmol(historic_records[i].glucose_value);
        double glucose = cgm::calculate_glucose_mmol(test_records[i]);
        int x = graph_x + static_cast<int>(i * x_scale);
        int y = graph_y + graph_height - static_cast<int>((glucose - min_glucose) * y_scale);
        y = std::clamp(y, graph_y, graph_y + graph_height);

        if (prev_x >= 0 && prev_y >= 0) {
            display.drawLine(prev_x, prev_y, x, y, WHITE);
        }

        prev_x = x;
        prev_y = y;
    }

    // Plot current glucose point and line to last valid point
    int x_current = graph_x + graph_width;
    int y_current = graph_y + graph_height - static_cast<int>((current_glucose - min_glucose) * y_scale);
    y_current = std::clamp(y_current, graph_y, graph_y + graph_height);

    if (prev_x >= 0 && prev_y >= 0) {
        display.drawLine(prev_x, prev_y, x_current, y_current, WHITE);
    }

    display.drawCircle(x_current, y_current, 2, WHITE);

    // Draw high and low glucose thresholds
    const float high_threshold = 10.0;
    const float low_threshold = 4.0;

    int y_high = graph_y + graph_height - static_cast<int>((high_threshold - min_glucose) * y_scale);
    int y_low = graph_y + graph_height - static_cast<int>((low_threshold - min_glucose) * y_scale);
    y_high = std::clamp(y_high, graph_y, graph_y + graph_height);
    y_low = std::clamp(y_low, graph_y, graph_y + graph_height);

    for (int x = graph_x; x < graph_x + graph_width; x += 4) {
        display.drawPixel(x, y_high, WHITE);
        display.drawPixel(x, y_low, WHITE);
    }

    // Compute average (excluding errors)
    int valid_count = 0;
    double sum_glucose = current_glucose;
    // for (auto& record : historic_records) {
    for (auto& record : test_records) {
        // if (!record.has_error && record.glucose_value > 0) {
        //     sum_glucose += cgm::calculate_glucose_mmol(record.glucose_value);
        //     valid_count++;
        // }
        if (record > 0) {
            sum_glucose += cgm::calculate_glucose_mmol(record);
            valid_count++;
        }
    }
    valid_count++; // Include current value

    double avg_glucose = (valid_count > 0) ? sum_glucose / valid_count : 0.0;

    auto avg_str = std::format("{:.1f}", avg_glucose).c_str();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(avg_str, 0, 0, &x1, &y1, &w, &h);
    int x_avg = (graph_x + graph_width + SCREEN_WIDTH) / 2 - w / 2;
    // int x_avg = (graph_x + graph_width) - w - 2;
    int y_avg = (graph_y + graph_height) - h - 2;
    display.setCursor(x_avg, y_avg);
    display.print(avg_str);

    // Display 'avg' text above
    display.drawBitmap(x_avg, y_avg - 20, average_txt, 16, 16, SSD1306_WHITE);


    // display.clearDisplay();
    // display.cp437(true);

    // // If there are no historic records, display an error message
    // if (historic_records.empty()) {
    //     ESP_LOGE(TAG, "No historic records");
    //     display_error(display, "No historic records");
    // }

    // // Reverse historic_records to display the most recent record on the right
    // std::reverse(historic_records.begin(), historic_records.end());

    // // Display historic glucose values in debug mode
    // for (auto i = 0; i < historic_records.size(); i++) {
    //     double glucose = cgm::calculate_glucose_mmol(historic_records[i].glucose_value);
    //     ESP_LOGD(TAG, "Historic glucose %d: %.1f", i, glucose);
    // }

    // // Define allowed glucose range for display
    // // const double min_allowed_glucose = 0.0; // TODO: remove magic numbers
    // // const double max_allowed_glucose = 32.0;

    // // Calculate the maximum and minimum glucose values in the historic records
    // double max_glucose = 32.0;
    // double min_glucose = 0.0;
    // // double max_glucose = current_glucose;
    // // double min_glucose = current_glucose;
    // // for (auto& record : historic_records) {
    // //     auto glucose = cgm::calculate_glucose_mmol(record.glucose_value);
    // //     min_glucose = std::min(min_glucose, glucose);
    // //     max_glucose = std::max(max_glucose, glucose);
    // // }

    // // // Clamp the min and max glucose values to the allowed range
    // // min_glucose = std::max(min_glucose, min_allowed_glucose);
    // // max_glucose = std::min(max_glucose, max_allowed_glucose);

    // // // Ensure there's a minimum range to avoid a flat graph
    // // if (max_glucose - min_glucose < 1.0) {
    // //     max_glucose = min_glucose + 1.0;
    // // }

    // // Calculate the graph dimensions
    // const int graph_width = display.width() - 40;
    // const int graph_height = display.height() - 12;
    // const int graph_x = 10;
    // const int graph_y = 5;

    // // Draw the graph axes
    // display.drawLine(graph_x, graph_y + graph_height, graph_x, graph_y, WHITE);
    // display.drawLine(graph_x, graph_y + graph_height, graph_x + graph_width, graph_y + graph_height, WHITE);

    // // Plot the glucose values as a line graph
    // const int num_records = historic_records.size();
    // auto num_records_no_error = num_records;
    // const double x_scale = static_cast<double>(graph_width) / (num_records - 1);
    // const double y_scale = static_cast<double>(graph_height) / (max_glucose - min_glucose);
    // int prev_x = -1, prev_y = -1;
    // for (int i = 0; i < num_records - 1; i++) {
    //     // Skip this segment if the current or next record has an error (create blank space)
    //     if (historic_records[i].has_error || historic_records[i + 1].has_error) {
    //         num_records_no_error -= 1;
    //         continue;
    //     }

    //     double glucose = cgm::calculate_glucose_mmol(historic_records[i].raw_glucose);
    //     int x = graph_x + static_cast<int>(i * x_scale);
    //     int y = graph_y + graph_height - static_cast<int>((glucose - min_glucose) * y_scale);
    //     y = std::clamp(y, graph_y, graph_y + graph_height);

    //     if (prev_x >= 0 && prev_y >= 0) {
    //         display.drawLine(prev_x, prev_y, x, y, WHITE);
    //     }

    //     prev_x = x;
    //     prev_y = y;

    //     // int x1 = graph_x + static_cast<int>(i * x_scale);
    //     // int y1 = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[i].glucose_value) - min_glucose) * y_scale);
    //     // int x2 = graph_x + static_cast<int>((i + 1) * x_scale);
    //     // int y2 = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[i + 1].glucose_value) - min_glucose) * y_scale);

    //     // // Ensure y1 and y2 are within the graph's bounds
    //     // y1 = std::max(graph_y, std::min(graph_y + graph_height, y1));
    //     // y2 = std::max(graph_y, std::min(graph_y + graph_height, y2));

    //     // display.drawLine(x1, y1, x2, y2, WHITE);
    // }

    // // Plot the current glucose value
    // int x_current = graph_x + graph_width; // Plot it at the far right of the graph
    // int y_current = graph_y + graph_height - static_cast<int>((current_glucose - min_glucose) * y_scale);
    // y_current = std::clamp(y_current, graph_y, graph_y + graph_height);

    // if (prev_x >= 0 && prev_y >= 0) {
    //     display.drawLine(prev_x, prev_y, x_current, y_current, WHITE);
    // }

    // // Get the coordinates of the last point in historic_records
    // // int x_last = graph_x + static_cast<int>((num_records - 1) * x_scale);
    // // int y_last = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[num_records - 1].glucose_value) - min_glucose) * y_scale);

    // // // Ensure y_last is within the graph's bounds
    // // y_last = std::max(graph_y, std::min(graph_y + graph_height, y_last));

    // // // Draw a line from the last historic record to the current glucose value
    // // display.drawLine(x_last, y_last, x_current, y_current, WHITE);

    // display.drawCircle(x_current, y_current, 2, WHITE);


    // // Define high and low glucose thresholds
    // const float high_glucose_threshold = 10.0; // Example: 10.0 mmol/L
    // const float low_glucose_threshold = 4.0;  // Example: 4.0 mmol/L

    // // Calculate y positions for the thresholds
    // int y_high = graph_y + graph_height - static_cast<int>((high_glucose_threshold - min_glucose) * y_scale);
    // int y_low = graph_y + graph_height - static_cast<int>((low_glucose_threshold - min_glucose) * y_scale);

    // // Ensure y positions are within the graph's bounds
    // y_high = std::max(graph_y, std::min(graph_y + graph_height, y_high));
    // y_low = std::max(graph_y, std::min(graph_y + graph_height, y_low));

    // // Draw dotted line for high glucose threshold
    // for (int x = graph_x; x < graph_x + graph_width; x += 4) {
    //     display.drawPixel(x, y_high, WHITE); // Draw a pixel every 4 units
    // }

    // // Draw dotted line for low glucose threshold
    // for (int x = graph_x; x < graph_x + graph_width; x += 4) {
    //     display.drawPixel(x, y_low, WHITE); // Draw a pixel every 4 units
    // }

    // // Display average glucose value (bottom right)
    // int16_t x1, y1;
    // uint16_t w, h;
    
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    
    // auto average_glucose = (std::accumulate(historic_records.begin(), historic_records.end(), 0.0,
    //     [](double sum, const cgm::fram_record& record) {
    //         if (!record.has_error && record.glucose_value >= 0) {
    //             return sum + cgm::calculate_glucose_mmol(record.glucose_value);
    //         }
    //         return sum;
    //     }) + current_glucose) / num_records_no_error;

    // auto average_str = std::format("{:.1f}", average_glucose).c_str();
    // display.getTextBounds(average_str, 0, 0, &x1, &y1, &w, &h);
    // int x_average = graph_x + graph_width - w - 2; // Position x_average at the right side of the graph
    // x_average -= (w + 2) / 2; // Position x_average in the middle of the available space
    // int y_average = graph_y + graph_height - h - 2; // Position average_str at the bottom of the graph

    // display.setCursor(x_average, y_average);
    // display.print(average_str);


    display.display();
    return;
}

void display_error(Adafruit_SSD1306& display, std::optional<std::string> error_message) {
    int16_t x1, y1;
    uint16_t w, h;

    // Save the current display state to return to later
    uint8_t saved_buffer[SCREEN_WIDTH * ((SCREEN_HEIGHT + 7) / 8)];
    memcpy(saved_buffer, display.getBuffer(), sizeof(saved_buffer));
    
    display.clearDisplay();
    display.cp437(true);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    std::string error = std::format("{} Error! {}", static_cast<char>(0x11), static_cast<char>(0x10));
    display.getTextBounds(error.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.1));
    display.println(error.c_str());

    if (error_message.has_value()) {
        display.getTextBounds(error_message.value().c_str(), 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.5));
        display.println(error_message.value().c_str());
    } else {
        display.getTextBounds("No error message", 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.5));
        display.println("No error message");
    }

    display.setTextSize(1);
    display.getTextBounds("Please rescan sensor", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.9));
    display.println("Please rescan sensor");

    display.display();

    // countdown timer to allow user to read error before redirecting
    auto seconds_delay = 3;
    for (auto i = 0; i < seconds_delay; i++) {
        auto text = std::format("({})", seconds_delay - i).c_str();

        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.print(text);
        display.display();

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait for 1 second

        // clear the text, so the numbers don't overlap
        display.setTextColor(SSD1306_BLACK);
        display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
        display.fillRect(0, 0, w, h, SSD1306_BLACK);
    }

    // Restore the saved display state before error
    memcpy(display.getBuffer(), saved_buffer, sizeof(saved_buffer));
    display.display();

    return;
}
