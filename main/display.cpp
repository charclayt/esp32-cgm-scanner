#include <display.hpp>

#include <common_main.hpp>
#include <trend.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <stdint.h>
#include <format>

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

void draw_glucose_display(Adafruit_SSD1306& display, double glucose_level, cgm::glucose_trend trend) {
    display.clearDisplay();
    display.cp437(true);

    // Format glucose text
    char glucose_buffer[16];
    snprintf(glucose_buffer, sizeof(glucose_buffer), "%.1f", glucose_level);

    // Set glucose value on the left side
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor((SCREEN_WIDTH * 0.15), (SCREEN_HEIGHT / 2) - 9); // vertically centered
    display.println(glucose_buffer);

    // Draw seperation line
    display.drawLine(SCREEN_WIDTH * 0.85, SCREEN_HEIGHT, SCREEN_WIDTH * 0.85, 0, SSD1306_WHITE);

    int16_t trend_x = SCREEN_WIDTH * 0.9;
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

    // Display predicted glucose level (15 minutes) in bottom-right corner


    display.display();
}

void display_error(Adafruit_SSD1306& display, std::optional<std::string> error_message) {
    int16_t x1, y1;
    uint16_t w, h;
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.cp437(true);

    display.setTextSize(1);
    std::string error = std::format("{} Error! {}", static_cast<char>(0x11), static_cast<char>(0x10));
    display.getTextBounds(error.c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.1));
    display.println(error.c_str());

    if (error_message.has_value()) {
        display.setTextSize(1);
        display.getTextBounds(error_message.value().c_str(), 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.5));
        display.println(error_message.value().c_str());
    } else {
        display.setTextSize(1);
        display.getTextBounds("No error message", 0, 0, &x1, &y1, &w, &h);
        display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.5));
        display.println("No error message");
    }

    display.setTextSize(1);
    display.getTextBounds("Please rescan sensor", 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT * 0.9));
    display.println("Please rescan sensor");

    display.display();

    // delay to allow user to read error, then reset to default display
    vTaskDelay(3000 / portTICK_PERIOD_MS); // Wait for 3 seconds
    draw_default_display(display);
    // TODO: redirect to sensor display screen if sensor is present (or when BLE implemented...)
    // TODO: sometimes triggers twd... (maybe reset WDT here?)
}
