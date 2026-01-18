/*
 * buzzer.h - Buzzer feedback for Metal Detektor
 *
 * Giver audio feedback baseret på signal styrke.
 * Højere frekvens = stærkere signal = tættere på metal
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/* ============ FUNKTIONER ============ */

/*
 * Initialiserer buzzer pin og Timer2 til tone generation
 */
void buzzer_init(void);

/*
 * Opdater buzzer baseret på signal styrke
 * signal: signal værdi (0 = intet signal, højere = stærkere)
 *
 * Mapping:
 *   0       -> Ingen lyd
 *   1-50    -> Langsom beep (ca. 2 Hz)
 *   51-100  -> Medium beep (ca. 5 Hz)
 *   101-150 -> Hurtig beep (ca. 10 Hz)
 *   151+    -> Kontinuerlig tone
 */
void buzzer_update(uint16_t signal);

/*
 * Slå buzzer til/fra
 */
void buzzer_enable(uint8_t enabled);

/*
 * Spil en kort bekræftelses-beep (til kalibrering etc.)
 */
void buzzer_beep(uint16_t duration_ms);

#endif /* BUZZER_H */
