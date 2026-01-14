/**
 * @file buzzer.c
 * @brief Buzzer output control for audio feedback
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of piezo buzzer tone generation using bit-banging.
 *          Outputs on PB2 (Arduino D10).
 */

#include "buzzer.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>

void buzzer_init(void)
{
    /* Configure PB2 as output */
    BUZZER_DDR |= (1 << BUZZER_PIN);

    /* Start with buzzer off */
    BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms)
{
    /*
     * Calculate half-period in microseconds:
     * half_period = 1000000 / (2 * freq) = 500000 / freq
     */
    uint16_t half_period = 500000UL / freq_hz;

    /*
     * Calculate number of complete cycles:
     * cycles = freq * duration / 1000
     */
    uint16_t cycles = ((uint32_t)freq_hz * duration_ms) / 1000;

    /* Generate square wave */
    for (uint16_t i = 0; i < cycles; i++) {
        /* High phase */
        BUZZER_PORT |= (1 << BUZZER_PIN);
        for (uint16_t j = 0; j < half_period; j++) {
            _delay_us(1);
        }

        /* Low phase */
        BUZZER_PORT &= ~(1 << BUZZER_PIN);
        for (uint16_t j = 0; j < half_period; j++) {
            _delay_us(1);
        }
    }
}

void buzzer_complete_melody(void)
{
    /* Three-note completion melody */
    buzzer_beep(1000, 150);     /* First beep: 1kHz, 150ms */
    _delay_ms(100);             /* Pause */

    buzzer_beep(1000, 150);     /* Second beep: 1kHz, 150ms */
    _delay_ms(100);             /* Pause */

    buzzer_beep(1500, 300);     /* Final beep: 1.5kHz, 300ms (higher, longer) */
}
