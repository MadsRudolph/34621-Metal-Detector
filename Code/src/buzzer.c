/*
 * buzzer.c - Buzzer feedback for Metal Detektor
 *
 * Korte beeps der bliver hurtigere med signal styrke
 * Signal 0-50 svarer til 0-100% på displayet
 */

#include <avr/io.h>
#include <util/delay.h>
#include "include/buzzer.h"
#include "include/config.h"

/* --- Lokale variable --- */
static uint8_t buzzer_enabled = 1;
static uint8_t beep_counter = 0;
static uint8_t beep_on_timer = 0;

#define BEEP_DURATION 1  // Kort beep (~50ms)

/* --- Buzzer Init --- */
void buzzer_init(void) {
    DDRB |= (1 << BUZZER_PIN);
    TCCR2A = (1 << WGM21);
    TCCR2B = (1 << CS22) | (1 << CS21);
    OCR2A = 30;  // ~1kHz tone
    TCCR2A &= ~(1 << COM2A0);
    PORTB &= ~(1 << BUZZER_PIN);
}

/* --- Buzzer Update --- */
void buzzer_update(uint16_t signal) {
    if (!buzzer_enabled || signal < 1) {
        TCCR2A &= ~(1 << COM2A0);
        PORTB &= ~(1 << BUZZER_PIN);
        beep_counter = 0;
        beep_on_timer = 0;
        return;
    }

    // Hvis beep er aktiv, tæl ned
    if (beep_on_timer > 0) {
        beep_on_timer--;
        if (beep_on_timer == 0) {
            TCCR2A &= ~(1 << COM2A0);
            PORTB &= ~(1 << BUZZER_PIN);
        }
        return;
    }

    // Signal 0-50 = 0-100% på display
    // Beregn pause mellem beeps (lavere = hurtigere)
    uint8_t pause;

    if (signal >= 50) {
        // 100%+ kontinuerlig
        TCCR2A |= (1 << COM2A0);
        return;
    } else if (signal >= 40) {
        // 80-100%: meget hurtig
        pause = 1;
    } else if (signal >= 25) {
        // 50-80%: hurtig
        pause = 2;
    } else if (signal >= 15) {
        // 30-50%: medium
        pause = 4;
    } else if (signal >= 5) {
        // 10-30%: langsom
        pause = 8;
    } else {
        // 2-10%: meget langsom
        pause = 15;
    }

    beep_counter++;

    if (beep_counter >= pause) {
        beep_counter = 0;
        TCCR2A |= (1 << COM2A0);
        beep_on_timer = BEEP_DURATION;
    }
}

/* --- Buzzer Enable/Disable --- */
void buzzer_enable(uint8_t enabled) {
    buzzer_enabled = enabled;
    if (!enabled) {
        TCCR2A &= ~(1 << COM2A0);
        PORTB &= ~(1 << BUZZER_PIN);
    }
}

/* --- Bekræftelses beep --- */
void buzzer_beep(uint16_t duration_ms) {
    TCCR2A |= (1 << COM2A0);
    while (duration_ms > 0) {
        _delay_ms(1);
        duration_ms--;
    }
    TCCR2A &= ~(1 << COM2A0);
    PORTB &= ~(1 << BUZZER_PIN);
}

/* --- Dobbelt beep --- */
void buzzer_double_click(void) {
    TCCR2A |= (1 << COM2A0);
    _delay_ms(50);
    TCCR2A &= ~(1 << COM2A0);
    PORTB &= ~(1 << BUZZER_PIN);
    _delay_ms(50);
    TCCR2A |= (1 << COM2A0);
    _delay_ms(50);
    TCCR2A &= ~(1 << COM2A0);
    PORTB &= ~(1 << BUZZER_PIN);
}
