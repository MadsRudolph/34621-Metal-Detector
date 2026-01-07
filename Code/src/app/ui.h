/*
 * ui.h
 * Brugerinterface - knapper og buzzer
 *
 * Knapper:
 * - Pin 2 (PE4/INT4) - Start/Stop knap
 * - Pin 3 (PE5/INT5) - Kalibrer knap
 *
 * Buzzer:
 * - Pin 8 (PH5/OC4C) - PWM buzzer output
 */

#ifndef UI_H
#define UI_H

#include <stdint.h>

/* Knap events (sættes af ISR, ryddes af main) */
extern volatile uint8_t btn_start_pressed;
extern volatile uint8_t btn_calibrate_pressed;

/* Initialiser knapper og buzzer hardware */
void ui_init(void);

/* Poll knapper (kald fra main loop) */
void ui_poll_buttons(void);

/* Opdater buzzer baseret på signal styrke (0-100) */
void buzzer_update(uint8_t strength);

/* Buzzer kontrol */
void buzzer_on(void);
void buzzer_off(void);

/* Enkelt beep (til feedback) */
void buzzer_beep(uint16_t duration_ms);

#endif
