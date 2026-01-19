/**
 * ssd1306_driver.h
 * SSD1306 OLED Display Driver (128x64) for Arduino Mega 2560
 *
 * ATTRIBUTION:
 * ============
 * Original Author: osch
 * Based on: Adafruit_SSD1306 library (github.com/adafruit/Adafruit_SSD1306)
 * Adapted for: ATmega2560 using I2C address 0x78 for writing and 0x7A for reading
 *
 * Documentation References:
 * - SSD1306 Advanced Information Matrix (Apr 2008, Rev 1.1) - solomon-systech.com
 *   Describes all commands and data used for 128x64 dot matrix display
 * - DD-2864bY-3A Rev C (p. 19) - Initialization sequence
 * Both datasheets available in project repository
 *
 * Hardware Connection (Arduino Mega 2560):
 * - VCC: +5V (or +3.3V depending on module)
 * - GND: GND (Pin 26)
 * - SDA: Pin 20 (PD1)
 * - SCL: Pin 21 (PD0)
 *
 * Integration: Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech (DTU 34621 Metal Detector Project)
 */

#ifndef SSD1306_DRIVER_H
#define SSD1306_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

// ============================================================================
// DISPLAY SIZE CONFIGURATION
// ============================================================================

#define SSD1306_128_64
//#define SSD1306_128_32
//#define SSD1306_96_16

#if defined SSD1306_128_64 && defined SSD1306_128_32
#error "Only one SSD1306 display can be specified at once in ssd1306_driver.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32 && !defined SSD1306_96_16
#error "At least one SSD1306 display must be specified in ssd1306_driver.h"
#endif

#if defined SSD1306_128_64
#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 32
#endif
#if defined SSD1306_96_16
#define SSD1306_LCDWIDTH                  96
#define SSD1306_LCDHEIGHT                 16
#endif

// ============================================================================
// SSD1306 COMMAND DEFINITIONS (from datasheet p. 28-32)
// ============================================================================

#define SSD1306_SETCONTRAST             0x81
#define SSD1306_DISPLAYALLON_RESUME     0xA4
#define SSD1306_DISPLAYALLON            0xA5
#define SSD1306_NORMALDISPLAY           0xA6
#define SSD1306_INVERTDISPLAY           0xA7
#define SSD1306_DISPLAYOFF              0xAE
#define SSD1306_DISPLAYON               0xAF
#define SSD1306_SETDISPLAYOFFSET        0xD3
#define SSD1306_SETCOMPINS              0xDA
#define SSD1306_SETVCOMDETECT           0xDB
#define SSD1306_SETDISPLAYCLOCKDIV      0xD5
#define SSD1306_SETPRECHARGE            0xD9
#define SSD1306_SETMULTIPLEX            0xA8
#define SSD1306_SETLOWCOLUMN            0x00
#define SSD1306_SETHIGHCOLUMN           0x10
#define SSD1306_SETSTARTLINE            0x40
#define SSD1306_MEMORYMODE              0x20
#define SSD1306_COLUMNADDR              0x21
#define SSD1306_PAGEADDR                0x22
#define SSD1306_COMSCANINC              0xC0
#define SSD1306_COMSCANDEC              0xC8
#define SSD1306_SEGREMAP                0xA0
#define SSD1306_CHARGEPUMP              0x8D
#define SSD1306_EXTERNALVCC             0x1
#define SSD1306_SWITCHCAPVCC            0x2

// Scrolling Commands
#define SSD1306_ACTIVATE_SCROLL         0x2F
#define SSD1306_DEACTIVATE_SCROLL       0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL  0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A

// ============================================================================
// COLOR DEFINITIONS
// ============================================================================

#define BLACK       0
#define WHITE       1
#define INVERSE     2

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

typedef uint8_t bitmap_t[8][128];

// ============================================================================
// PUBLIC FUNCTION DECLARATIONS
// ============================================================================

// Core Display Functions
void InitializeDisplay(void);
void displayOn(void);
void displayOff(void);
void clear_display(void);
void reset_display(void);

// Text Display Functions
void sendStr(char *string);
void sendStrXY(char *string, int X, int Y);
void SendChar(unsigned char data);
void sendCharXY(unsigned char data, int X, int Y);

// Large Number Display Functions
void printBigTime(char *string);
void printBigNumber(char string, int X, int Y);

// Position Control
void setXY(unsigned char row, unsigned char col);
void ssd1306_setpos(uint8_t x, uint8_t y);
void setPageAddress(void);
void setColAddress(void);

// Graphics Functions
void bmp(bitmap_t b);
void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
void drawPixel(int16_t x, int16_t y, uint16_t color);

// Scrolling Functions
void startscrollright(uint8_t start, uint8_t stop);
void startscrollleft(uint8_t start, uint8_t stop);
void startscrolldiagright(uint8_t start, uint8_t stop);
void startscrolldiagleft(uint8_t start, uint8_t stop);
void stopscroll(void);

// Display Control
void dim(bool dim);
void invertDisplay(uint8_t i);
void print_fonts(void);

// Graphical Drawing Functions
void draw_icon(const uint8_t *icon, uint8_t page, uint8_t col);
void draw_hbar(uint8_t page, uint8_t col, uint8_t width, uint8_t fill);
void fill_rect(uint8_t page, uint8_t col, uint8_t width, uint8_t pattern);
void draw_centered_text(char *str, uint8_t page);

#endif // SSD1306_DRIVER_H
