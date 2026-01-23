/*
 * jingle.c - Opstartslyd for Metal Detektor
 */

#include <avr/io.h>
#include <util/delay.h>
#include "include/jingle.h"
#include "include/config.h"

/* --- Tone funktion --- */
static void tone(uint16_t freq, uint16_t ms) {
    uint16_t half = 500000UL / freq;
    uint32_t cycles = ((uint32_t)freq * ms) / 1000;

    for (uint32_t i = 0; i < cycles; i++) {
        PORTB |= (1 << BUZZER_PIN);
        for (uint16_t d = 0; d < half; d++) _delay_us(1);
        PORTB &= ~(1 << BUZZER_PIN);
        for (uint16_t d = 0; d < half; d++) _delay_us(1);
    }
}

/* --- Opstartslyd --- */
void play_startup_jingle(void) {
    // Stigende power-up lyd
    tone(400, 80);
    tone(500, 80);
    tone(600, 80);
    tone(800, 150);
    _delay_ms(50);
    tone(1000, 200);
}
