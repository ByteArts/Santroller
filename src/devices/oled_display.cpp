#include "devices/oled_display.hpp"
#include <cstring>
#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Basic 5x7 ASCII font bitmap table for characters ' ' (32) through 'z' (122).
static const uint8_t FONT_5X7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '\''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x26, 0x49, 0x49, 0x49, 0x32}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // '['
    {0x02, 0x04, 0x08, 0x10, 0x20}, // '\\'
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ']'
    {0x04, 0x02, 0x01, 0x02, 0x04}, // '^'
    {0x40, 0x40, 0x40, 0x40, 0x40}, // '_'
    {0x00, 0x01, 0x02, 0x04, 0x00}, // '`'
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a'
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b'
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c'
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd'
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e'
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f'
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 'g'
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h'
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i'
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j'
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 'k'
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l'
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm'
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n'
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o'
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p'
    {0x08, 0x14, 0x14, 0x14, 0x7C}, // 'q'
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r'
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's'
    {0x04, 0x3E, 0x44, 0x40, 0x20}, // 't'
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u'
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v'
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w'
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x'
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y'
    {0x44, 0x64, 0x54, 0x4C, 0x44}  // 'z'
};

OLEDDisplay::OLEDDisplay(proto_OLEDDisplayDevice device, uint16_t id)
    : Device(id),
      m_config{
          device.i2c.block == 0 ? i2c0 : i2c1,
          static_cast<uint8_t>(device.i2c.sda),
          static_cast<uint8_t>(device.i2c.scl),
          static_cast<uint8_t>(device.has_address ? device.address : 0x3C),
          static_cast<uint32_t>(device.i2c.clock)},
      m_device(device) {}

bool OLEDDisplay::init() {
    if ((m_device.i2c.block != 0 && m_device.i2c.block != 1) ||
        m_device.i2c.sda < 0 || m_device.i2c.sda > UINT8_MAX ||
        m_device.i2c.scl < 0 || m_device.i2c.scl > UINT8_MAX ||
        (m_device.has_address && m_device.address > 0x7F) ||
        m_device.i2c.clock <= 0 ||
        m_config.sda_pin >= NUM_BANK0_GPIOS ||
        m_config.scl_pin >= NUM_BANK0_GPIOS ||
        m_config.sda_pin == m_config.scl_pin) {
        return false;
    }

    if (!m_i2c_initialized) {
        if (i2c_init(m_config.i2c_inst, m_config.baudrate) == 0) {
            return false;
        }
        gpio_set_function(m_config.sda_pin, GPIO_FUNC_I2C);
        gpio_set_function(m_config.scl_pin, GPIO_FUNC_I2C);
        gpio_pull_up(m_config.sda_pin);
        gpio_pull_up(m_config.scl_pin);
        m_i2c_initialized = true;
    }

    // SSD1306 Initialization sequence for 128x32 OLED panel
    static const uint8_t init_cmds[] = {
        0xAE,       // Display OFF
        0xD5, 0x80, // Set Display Clock Divide Ratio / Oscillator Frequency
        0xA8, 0x1F, // Set Multiplex Ratio (32 lines - 1 = 0x1F)
        0xD3, 0x00, // Set Display Offset (0)
        0x40,       // Set Start Line (0)
        0x8D, 0x14, // Enable Charge Pump
        0x20, 0x00, // Memory Addressing Mode: Horizontal
        0xA1,       // Segment Re-map (col 127 mapped to SEG0)
        0xC8,       // COM Output Scan Direction (remapped)
        0xDA, 0x02, // COM Pins Hardware Configuration (Sequential)
        0x81, 0x8F, // Set Contrast Control
        0xD9, 0xF1, // Set Pre-charge Period
        0xDB, 0x40, // Set VCOMH Deselect Level
        0xA4,       // Entire Display ON (Resume to RAM content)
        0xA6,       // Normal Display (not inverted)
        0xAF        // Display ON
    };

    if (!send_command_list(init_cmds, sizeof(init_cmds))) {
        m_initialized = false;
        return false;
    }

    m_initialized = true;
    clear();
    return flush();
}

void OLEDDisplay::begin() {
    m_next_init_attempt_ms = to_ms_since_boot(get_absolute_time()) + 1000;
    m_i2c_error_reported = !init();
    if (m_i2c_error_reported) {
        printf("OLED display initialization failed on I2C bus %d\r\n", m_device.i2c.block);
    }
}

void OLEDDisplay::end(bool full) {
    (void)full;
    if (m_initialized && !send_command(0xAE)) {
        printf("OLED display shutdown command failed\r\n");
    }
    m_initialized = false;
}

void OLEDDisplay::update(bool full_poll, bool send_events) {
    (void)full_poll;
    (void)send_events;

    if (!m_initialized) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (static_cast<int32_t>(now - m_next_init_attempt_ms) >= 0) {
            m_next_init_attempt_ms = now + 1000;
            if (init()) {
                m_i2c_error_reported = false;
            } else if (!m_i2c_error_reported) {
                printf("OLED display initialization retry failed on I2C bus %d\r\n", m_device.i2c.block);
                m_i2c_error_reported = true;
            }
        }
        return;
    }

    if (m_dirty) {
        if (!flush()) {
            if (!m_i2c_error_reported) {
                printf("OLED display frame transfer failed on I2C bus %d\r\n", m_device.i2c.block);
                m_i2c_error_reported = true;
            }
        } else {
            m_i2c_error_reported = false;
        }
    }
}

void OLEDDisplay::clear() {
    std::memset(m_buffer, 0, sizeof(m_buffer));
    m_dirty = true;
}

bool OLEDDisplay::flush() {
    if (!m_initialized) {
        return false;
    }

    // Set page address range (0 to 3) and column address range (0 to 127)
    static const uint8_t set_bounds[] = {
        0x21, 0x00, 0x7F, // Column start: 0, end: 127
        0x22, 0x00, 0x03  // Page start: 0, end: 3
    };
    if (!send_command_list(set_bounds, sizeof(set_bounds)) ||
        !send_data(m_buffer, sizeof(m_buffer))) {
        return false;
    }

    m_dirty = false;
    return true;
}

void OLEDDisplay::draw_pixel(int16_t x, int16_t y, bool color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
        return; // Clipping out-of-bounds
    }

    size_t index = x + (y / 8) * WIDTH;
    uint8_t bit = 1 << (y % 8);

    if (color) {
        m_buffer[index] |= bit;
    } else {
        m_buffer[index] &= ~bit;
    }
    m_dirty = true;
}

void OLEDDisplay::draw_char(int16_t x, int16_t y, char c, bool color) {
    if (c < 32 || c > 'z') {
        c = '?';
    }
    const uint8_t* char_data = FONT_5X7[c - 32];

    for (int col = 0; col < 5; ++col) {
        uint8_t line = char_data[col];
        for (int row = 0; row < 7; ++row) {
            if (line & (1 << row)) {
                draw_pixel(x + col, y + row, color);
            }
        }
    }
}

void OLEDDisplay::draw_string(int16_t x, int16_t y, const char* str, bool color) {
    if (str == nullptr) {
        return;
    }

    int16_t cursor_x = x;
    while (*str) {
        if (cursor_x + 5 >= WIDTH) {
            break; // Stop at right screen edge
        }
        draw_char(cursor_x, y, *str, color);
        cursor_x += 6; // 5 pixels character width + 1 pixel spacing
        str++;
    }
}

void OLEDDisplay::draw_status(const char* status, uint8_t player_num) {
    clear();

    // Top row: Header
    draw_string(0, 0, "SANTROLLER");

    // Divider line
    for (int x = 0; x < WIDTH; ++x) {
        draw_pixel(x, 10, true);
    }

    // Status message
    draw_string(0, 14, status);

    // Player Number indicator on the right side
    if (player_num >= 1 && player_num <= 4) {
        char p_str[5] = "P1";
        p_str[1] = '0' + player_num;
        draw_string(110, 24, p_str);
    }

    m_dirty = true;
}

bool OLEDDisplay::using_pin(uint8_t pin) {
    return pin == m_device.i2c.sda || pin == m_device.i2c.scl;
}

bool OLEDDisplay::send_command(uint8_t cmd) {
    uint8_t buf[2] = {0x00, cmd}; // 0x00 Co=0, D/C#=0
    return i2c_write_blocking(m_config.i2c_inst, m_config.i2c_address, buf, sizeof(buf), false) ==
           static_cast<int>(sizeof(buf));
}

bool OLEDDisplay::send_command_list(const uint8_t* cmds, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!send_command(cmds[i])) {
            return false;
        }
    }
    return true;
}

bool OLEDDisplay::send_data(const uint8_t* data, size_t len) {
    if (len > FRAMEBUFFER_SIZE) {
        return false;
    }

    static constexpr size_t DATA_CHUNK_SIZE = 128;
    uint8_t tx_buf[DATA_CHUNK_SIZE + 1];
    size_t offset = 0;
    while (offset < len) {
        size_t chunk_size = len - offset;
        if (chunk_size > DATA_CHUNK_SIZE) {
            chunk_size = DATA_CHUNK_SIZE;
        }

        tx_buf[0] = 0x40;
        std::memcpy(&tx_buf[1], &data[offset], chunk_size);
        if (i2c_write_blocking(m_config.i2c_inst, m_config.i2c_address, tx_buf, chunk_size + 1, false) !=
            static_cast<int>(chunk_size + 1)) {
            return false;
        }
        offset += chunk_size;
    }
    return true;
}
