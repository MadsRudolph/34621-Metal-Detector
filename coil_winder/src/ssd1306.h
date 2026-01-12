/**
 * @file ssd1306.h
 * @brief SSD1306 OLED display driver (128x64)
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Driver for SSD1306-based 128x64 pixel OLED display over I2C.
 *          Uses a local framebuffer that is transferred to display on update.
 *          Includes 5x7 pixel font and basic drawing primitives.
 *
 *          Display organization:
 *          - 128 columns x 64 rows
 *          - 8 pages (rows 0-7, 8-15, ... 56-63)
 *          - Each byte controls 8 vertical pixels (LSB = top)
 *
 * @note I2C address is 0x3C (7-bit), configured in config.h
 */

#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize SSD1306 display
 *
 * @details Sends initialization command sequence to configure:
 *          - Display off during init
 *          - Clock divider and multiplex ratio
 *          - Charge pump enabled (for 3.3V operation)
 *          - Horizontal addressing mode
 *          - Segment/COM remapping for correct orientation
 *          - Contrast and precharge settings
 *          - Display on
 *
 * @note Includes 100ms startup delay for display power stabilization.
 */
void ssd1306_init(void);

/**
 * @brief Clear display buffer
 *
 * @details Sets all pixels in the framebuffer to off (black).
 *          Call ssd1306_update() to transfer to display.
 */
void ssd1306_clear(void);

/**
 * @brief Update display with buffer contents
 *
 * @details Transfers the entire 1024-byte framebuffer to the display
 *          via I2C. This is a blocking operation that takes ~2.5ms at 400kHz.
 */
void ssd1306_update(void);

/**
 * @brief Set a single pixel
 *
 * @details Sets or clears a pixel in the framebuffer.
 *          Out-of-bounds coordinates are silently ignored.
 *
 * @param x X coordinate (0-127, left to right)
 * @param y Y coordinate (0-63, top to bottom)
 * @param on true = pixel on (white), false = pixel off (black)
 */
void ssd1306_set_pixel(uint8_t x, uint8_t y, bool on);

/**
 * @brief Draw a character at position
 *
 * @details Renders a single ASCII character using the built-in 5x7 font.
 *          Supports characters 32-122 (space through 'z').
 *          Invalid characters are replaced with '?'.
 *
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param c ASCII character to draw
 * @param scale Scale factor (1 = 5x7 pixels, 2 = 10x14, etc.)
 */
void ssd1306_draw_char(uint8_t x, uint8_t y, char c, uint8_t scale);

/**
 * @brief Draw a string at position
 *
 * @details Renders a null-terminated string horizontally.
 *          Characters are spaced 6*scale pixels apart.
 *
 * @param x X coordinate of first character
 * @param y Y coordinate
 * @param str Null-terminated ASCII string
 * @param scale Scale factor for all characters
 */
void ssd1306_draw_string(uint8_t x, uint8_t y, const char *str, uint8_t scale);

/**
 * @brief Draw an integer value
 *
 * @details Renders a number as zero-padded decimal digits.
 *          Negative numbers are converted to positive (no sign displayed).
 *
 * @param x X coordinate
 * @param y Y coordinate
 * @param num Number to display (-32768 to 32767)
 * @param digits Number of digits to display (zero-padded)
 * @param scale Scale factor
 */
void ssd1306_draw_int(uint8_t x, uint8_t y, int16_t num, uint8_t digits, uint8_t scale);

/**
 * @brief Draw a rectangle
 *
 * @details Draws an axis-aligned rectangle, either outline or filled.
 *
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Width in pixels
 * @param h Height in pixels
 * @param fill true = filled rectangle, false = outline only
 */
void ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool fill);

/**
 * @brief Draw a progress bar
 *
 * @details Draws a rectangular progress bar with outline and fill.
 *          Fill width is proportional to value/max_val.
 *
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param w Total width in pixels
 * @param h Height in pixels
 * @param value Current progress value
 * @param max_val Maximum value (100% fill)
 */
void ssd1306_draw_progress(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                           uint16_t value, uint16_t max_val);

#endif /* SSD1306_H */
