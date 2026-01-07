/*
 * display.h
 * Simpel wrapper til OLED display
 *
 * Skjuler SSD1306 driver detaljer fra applikationskoden.
 * Display er 16 karakterer bredt x 8 rækker højt.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* Initialiser I2C og OLED display - kald én gang ved opstart */
void display_init(void);

/* Ryd hele skærmen */
void display_clear(void);

/* Udskriv tekst på position (række 0-7, kolonne 0-15) */
void display_text(uint8_t row, uint8_t col, const char *text);

/* Udskriv unsigned tal på position (til ADC værdier osv.) */
void display_number(uint8_t row, uint8_t col, uint16_t value);

/* ===== Skærmbilleder ===== */

/* Splash screen ved opstart */
void screen_splash(void);

/* Idle skærm (detektor stoppet) */
void screen_idle(void);

/* Kalibrerings skærm */
void screen_calibrating(void);

/* Detektion skærm med live data */
void screen_detection(uint8_t strength, int16_t phase, uint8_t metal_type);

/* Tegn styrke-bar (0-100) */
void display_bar(uint8_t row, uint8_t col, uint8_t width, uint8_t value);

#endif
