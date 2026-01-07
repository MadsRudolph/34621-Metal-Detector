/*
 * display.h
 * Simple wrapper for OLED display
 *
 * Hides the SSD1306 driver details from application code.
 * Display is 16 characters wide x 8 rows tall.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* Initialize I2C and OLED display - call once at startup */
void display_init(void);

/* Clear entire screen */
void display_clear(void);

/* Print text at position (row 0-7, col 0-15) */
void display_text(uint8_t row, uint8_t col, const char *text);

/* Print unsigned number at position (for ADC values etc) */
void display_number(uint8_t row, uint8_t col, uint16_t value);

#endif
