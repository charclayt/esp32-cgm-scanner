#ifndef __COMMON_MAIN_HPP__
#define __COMMON_MAIN_HPP__

// TODO: rename this file

// ESP-32 <-> PN5180 pinout mapping
constexpr uint8_t PN5180_NSS = 12;  // GPIO12
constexpr uint8_t PN5180_BUSY = 13;  // GPIO13
constexpr uint8_t PN5180_RST = 14;  // GPIO14

// ESP-32 <-> SSD1306 pinout mapping
constexpr uint8_t OLED_MOSI = 5;
constexpr uint8_t OLED_CLK = 4;
constexpr uint8_t OLED_DC = 1;
constexpr uint8_t OLED_CS = 2;
constexpr uint8_t OLED_RESET = 3;

constexpr uint8_t SCREEN_WIDTH = 128; // OLED display width, in pixels
constexpr uint8_t SCREEN_HEIGHT = 64; // OLED display height, in pixels

#endif // __COMMON_MAIN_HPP__
