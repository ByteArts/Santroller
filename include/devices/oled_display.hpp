#pragma once

#include <cstdint>
#include <cstddef>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

namespace santroller {

/**
 * @brief Configuration parameters for the SSD1306 OLED display.
 */
struct OLEDConfig {
    i2c_inst_t* i2c_inst = i2c0;  ///< Hardware I2C instance (i2c0 or i2c1)
    uint8_t sda_pin = 4;           ///< GPIO pin for SDA
    uint8_t scl_pin = 5;           ///< GPIO pin for SCL
    uint8_t i2c_address = 0x3C;    ///< Standard SSD1306 I2C address (0x3C or 0x3D)
    uint32_t baudrate = 400000;    ///< I2C clock speed (400 kHz Fast Mode)
};

/**
 * @brief Device driver class for a 128x32 I2C SSD1306 OLED display.
 */
class OLEDDisplay {
public:
    static constexpr uint8_t WIDTH = 128;
    static constexpr uint8_t HEIGHT = 32;
    static constexpr uint8_t PAGES = HEIGHT / 8; // 4 pages
    static constexpr size_t BUFFER_SIZE = WIDTH * PAGES; // 512 bytes

    OLEDDisplay() = default;
    ~OLEDDisplay() = default;

    /**
     * @brief Initializes the hardware I2C peripheral and sends the SSD1306 setup commands.
     * @param config The I2C and GPIO configuration settings.
     * @return true if initialization succeeded, false on I2C error.
     */
    bool init(const OLEDConfig& config);

    /**
     * @brief Periodic update function called from the main loop.
     * Re-renders the display if the frame buffer is dirty.
     */
    void update();

    /**
     * @brief Clears the internal frame buffer (all pixels off).
     */
    void clear();

    /**
     * @brief Flushes the frame buffer out to the display over I2C.
     */
    void flush();

    /**
     * @brief Sets an individual pixel in the frame buffer.
     * @param x Horizontal coordinate (0..127)
     * @param y Vertical coordinate (0..31)
     * @param color true = ON (white), false = OFF (black)
     */
    void draw_pixel(int16_t x, int16_t y, bool color);

    /**
     * @brief Draws a character using an embedded 5x7 font.
     */
    void draw_char(int16_t x, int16_t y, char c, bool color = true);

    /**
     * @brief Draws a null-terminated string using an embedded 5x7 font.
     */
    void draw_string(int16_t x, int16_t y, const char* str, bool color = true);

    /**
     * @brief High-level helper to update status display elements.
     * @param status String representing current connection status (e.g., "Connected", "Searching")
     * @param player_num Assigned player number (1 to 4)
     */
    void draw_status(const char* status, uint8_t player_num);

private:
    OLEDConfig m_config;
    uint8_t m_buffer[BUFFER_SIZE] = {0};
    bool m_dirty = false;

    void send_command(uint8_t cmd);
    void send_command_list(const uint8_t* cmds, size_t count);
    void send_data(const uint8_t* data, size_t len);
};

} // namespace santroller
