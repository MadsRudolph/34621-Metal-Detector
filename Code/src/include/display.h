/*
 * display.h - OLED display funktioner for Metal Detektor
 *
 * Viser DFT resultater og debug information på SSD1306 OLED display
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* --- Funktioner --- */

/*
 * Vis DFT resultater på display
 * Viser: Re, Im, Magnitude, Fase, Metal type
 * Hvis pauset, vises pause-besked
 */
void display_dft(void);

/*
 * Vis kalibrerings-bekræftelse
 */
void display_calibrated(void);

/*
 * Vis splash screen ved opstart
 */
void display_splash(void);

#endif /* DISPLAY_H */
