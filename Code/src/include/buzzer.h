/*
 * buzzer.h - Buzzer feedback for Metal Detektor
 *
 * Korte beeps der bliver hurtigere med signal styrke
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/* Initialiserer buzzer pin og Timer2 */
void buzzer_init(void);

/* Opdater buzzer baseret på signal styrke */
void buzzer_update(uint16_t signal);

/* Slå buzzer til/fra */
void buzzer_enable(uint8_t enabled);

/* Spil en kort bekræftelses-beep */
void buzzer_beep(uint16_t duration_ms);

/* Dobbelt-beep */
void buzzer_double_click(void);

#endif /* BUZZER_H */
