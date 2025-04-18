#include <display_utils.hpp>

#include <common_main.hpp>

#include <calculations.hpp>
#include <fram.hpp>
#include <trend.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdint.h>
#include <format>

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

    // Draw seperation line
    display.drawLine(SCREEN_WIDTH * 0.8, SCREEN_HEIGHT, SCREEN_WIDTH * 0.8, 0, SSD1306_WHITE);

    int16_t trend_x = SCREEN_WIDTH * 0.85;
    int16_t trend_y = SCREEN_HEIGHT / 3;

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

    // TODO: Display predicted glucose level (15 minutes) in bottom-right corner
    display.setCursor(SCREEN_WIDTH * 0.83, SCREEN_HEIGHT * 0.75);
    display.setTextSize(1);
    // display.println("predicted");
    // display.setCursor(SCREEN_WIDTH * 0.9, SCREEN_HEIGHT * 0.85);
    display.println(std::format("{:.1f}", predicted_glucose).c_str());

    display.display();
}

void draw_historic_display(Adafruit_SSD1306& display, double current_glucose, std::vector<cgm::fram_record>& historic_records) {
    display.clearDisplay();
    display.cp437(true);

    // If there are no historic records, display an error message
    if (historic_records.empty()) {
        display_error(display, "No historic records");
    }

    // Reverse historic_records to display the most recent record on the right
    std::reverse(historic_records.begin(), historic_records.end());

    // Display historic glucose values in debug mode
    for (auto i = 0; i < historic_records.size(); i++) {
        double glucose = cgm::calculate_glucose_mmol(historic_records[i]);
        ESP_LOGD(TAG, "Historic glucose %d: %.2f", i, glucose);
    }

    // Define allowed glucose range for display
    const double min_allowed_glucose = 0.0; // TODO: remove magic numbers
    const double max_allowed_glucose = 21.0;

    // Calculate the maximum and minimum glucose values in the historic records
    double max_glucose = current_glucose;
    double min_glucose = current_glucose;
    for (auto& record : historic_records) {
        auto glucose = cgm::calculate_glucose_mmol(record);
        min_glucose = std::min(min_glucose, glucose);
        max_glucose = std::max(max_glucose, glucose);
    }

    // Clamp the min and max glucose values to the allowed range
    min_glucose = std::max(min_glucose, min_allowed_glucose);
    max_glucose = std::min(max_glucose, max_allowed_glucose);

    // Ensure there's a minimum range to avoid a flat graph
    if (max_glucose - min_glucose < 1.0) {
        max_glucose = min_glucose + 1.0;
    }

    // Calculate the graph dimensions
    const int graph_width = display.width() - 40;
    const int graph_height = display.height() - 12;
    const int graph_x = 10;
    const int graph_y = 5;

    // Draw the graph axes
    display.drawLine(graph_x, graph_y + graph_height, graph_x, graph_y, WHITE);
    display.drawLine(graph_x, graph_y + graph_height, graph_x + graph_width, graph_y + graph_height, WHITE);

    // Plot the glucose values as a line graph
    const int num_records = historic_records.size();
    const double x_scale = static_cast<double>(graph_width) / (num_records - 1);
    const double y_scale = static_cast<double>(graph_height) / (max_glucose - min_glucose);
    for (int i = 0; i < num_records - 1; i++) {
        // Skip this segment if the current or next record has an error (create blank space)
        if (historic_records[i].has_error || historic_records[i + 1].has_error) {
            continue;
        }

        int x1 = graph_x + static_cast<int>(i * x_scale);
        int y1 = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[i]) - min_glucose) * y_scale);
        int x2 = graph_x + static_cast<int>((i + 1) * x_scale);
        int y2 = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[i + 1]) - min_glucose) * y_scale);

        // Ensure y1 and y2 are within the graph's bounds
        y1 = std::max(graph_y, std::min(graph_y + graph_height, y1));
        y2 = std::max(graph_y, std::min(graph_y + graph_height, y2));

        display.drawLine(x1, y1, x2, y2, WHITE);
    }

    // Plot the current glucose value
    int x_current = graph_x + graph_width; // Plot it at the far right of the graph
    int y_current = graph_y + graph_height - static_cast<int>((current_glucose - min_glucose) * y_scale);

    // Get the coordinates of the last point in historic_records
    int x_last = graph_x + static_cast<int>((num_records - 1) * x_scale);
    int y_last = graph_y + graph_height - static_cast<int>((cgm::calculate_glucose_mmol(historic_records[num_records - 1]) - min_glucose) * y_scale);

    // Ensure y_last is within the graph's bounds
    y_last = std::max(graph_y, std::min(graph_y + graph_height, y_last));

    // Draw a line from the last historic record to the current glucose value
    display.drawLine(x_last, y_last, x_current, y_current, WHITE);

    display.drawCircle(x_current, y_current, 2, WHITE);


    // Define high and low glucose thresholds
    const float high_glucose_threshold = 10.0; // Example: 10.0 mmol/L
    const float low_glucose_threshold = 4.0;  // Example: 4.0 mmol/L

    // Calculate y positions for the thresholds
    int y_high = graph_y + graph_height - static_cast<int>((high_glucose_threshold - min_glucose) * y_scale);
    int y_low = graph_y + graph_height - static_cast<int>((low_glucose_threshold - min_glucose) * y_scale);

    // Ensure y positions are within the graph's bounds
    y_high = std::max(graph_y, std::min(graph_y + graph_height, y_high));
    y_low = std::max(graph_y, std::min(graph_y + graph_height, y_low));

    // Draw dotted line for high glucose threshold
    for (int x = graph_x; x < graph_x + graph_width; x += 4) {
        display.drawPixel(x, y_high, WHITE); // Draw a pixel every 4 units
    }

    // Draw dotted line for low glucose threshold
    for (int x = graph_x; x < graph_x + graph_width; x += 4) {
        display.drawPixel(x, y_low, WHITE); // Draw a pixel every 4 units
    }


    display.display();
}

void display_error(Adafruit_SSD1306& display, std::optional<std::string> error_message) {
    int16_t x1, y1;
    uint16_t w, h;
    
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

    // delay to allow user to read error, then reset to default display
    draw_default_display(display);
    // TODO: redirect to sensor display screen if sensor is present (or when BLE implemented...)
    // TODO: sometimes triggers wdt... (maybe reset WDT here?)
}
