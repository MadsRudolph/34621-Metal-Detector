/*
 * display.h - OLED display funktioner for Metal Detektor
 *
 * Viser DFT resultater og debug information på SSD1306 OLED display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* ============ GLOBALE VARIABLE ============ */
// Display mode flag
extern uint8_t show_debug;      // 0=DFT visning, 1=Debug visning

/* ============ FUNKTIONER ============ */

/*
 * Vis DFT resultater på display
 * Viser: Re, Im, Magnitude, Fase, Metal type
 * Hvis pauset, vises pause-besked
 */
void display_dft(void);

/*
 * Vis debug information på display
 * Viser: Rå ADC værdi
 */
void display_debug(void);

/*
 * Vis kalibrerings-bekræftelse
 */
void display_calibrated(void);

/*
 * Toggle mellem DFT og debug visning
 */
void toggle_display_mode(void);

/*
 * Vis splash screen ved opstart
 */
void display_splash(void);

#endif /* DISPLAY_H */
