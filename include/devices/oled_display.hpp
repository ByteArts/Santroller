#pragma once

#include <cstdint>
#include <cstddef>
#include "devices/base.hpp"
#include "device.pb.h"
#include "hardware/i2c.h"

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
 * @brief Device driver for a 128x32 I2C SSD1306 OLED display.
 */
class OLEDDisplay : public Device {
public:
    static constexpr uint8_t WIDTH = 128;
    static constexpr uint8_t HEIGHT = 32;
    static constexpr uint8_t PAGES = HEIGHT / 8;
    static constexpr size_t FRAMEBUFFER_SIZE = WIDTH * PAGES;

    OLEDDisplay(proto_OLEDDisplayDevice device, uint16_t id);
    ~OLEDDisplay() = default;

    /**
     * @brief Initializes the display using the configured I2C bus.
     * @return true if initialization and the initial frame transfer succeed.
     */
    bool init();

    void begin() override;
    void end(bool full) override;

    /**
     * @brief Periodic update function called from the main loop.
     * Retries initialization or transfers a dirty frame.
     */
    void update(bool full_poll, bool send_events) override;

    bool using_pin(uint8_t pin) override;

    /**
     * @brief Clears the internal frame buffer (all pixels off).
     */
    void clear();

    /**
     * @brief Flushes the frame buffer out to the display over I2C.
     * @return true if the complete frame was transferred.
     */
    bool flush();

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
    proto_OLEDDisplayDevice m_device;
    uint8_t m_buffer[FRAMEBUFFER_SIZE] = {0};
    bool m_dirty = false;
    bool m_i2c_initialized = false;
    bool m_initialized = false;
    bool m_i2c_error_reported = false;
    uint32_t m_next_init_attempt_ms = 0;

    bool send_command(uint8_t cmd);
    bool send_command_list(const uint8_t* cmds, size_t count);
    bool send_data(const uint8_t* data, size_t len);
};
