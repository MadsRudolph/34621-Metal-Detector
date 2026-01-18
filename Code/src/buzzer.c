/*
 * buzzer.c - Buzzer feedback for Metal Detektor
 *
 * Bruger Timer2 til at generere tone på Pin 11 (PB3/OC2A)
 * Beep rate øges med signal styrke
 */

#include <avr/io.h>
#include <util/delay.h>
#include "buzzer.h"
#include "config.h"

/* ============ LOKALE VARIABLE ============ */
static uint8_t buzzer_enabled = 1;      // Buzzer aktiv som default
static uint8_t beep_counter = 0;        // Tæller til beep timing
static uint8_t tone_on = 0;             // Er tonen aktiv lige nu?

/* ============ FORWARD DECLARATIONS ============ */
static void buzzer_on(void);
static void buzzer_off(void);

/* ============ BUZZER INIT ============ */
void buzzer_init(void) {
    // Sæt buzzer pin som output
    DDRB |= (1 << BUZZER_PIN);

    // Timer2: CTC mode, toggle OC2A on compare match
    // Dette genererer en firkantbølge på Pin 11
    TCCR2A = (1 << COM2A0) | (1 << WGM21);  // Toggle OC2A, CTC mode

    // Prescaler 256: 16MHz / 256 = 62.5kHz timer clock
    // OCR2A = 31 giver: 62500 / (2 * 32) = ~977 Hz tone
    TCCR2B = (1 << CS22) | (1 << CS21);     // Prescaler 256

    OCR2A = 31;  // ~1kHz tone

    // Start med buzzer slukket
    buzzer_off();
}

/* ============ INTERN: TONE KONTROL ============ */
static void buzzer_on(void) {
    // Aktiver toggle mode for OC2A
    TCCR2A |= (1 << COM2A0);
    tone_on = 1;
}

static void buzzer_off(void) {
    // Deaktiver toggle, sæt pin lav
    TCCR2A &= ~(1 << COM2A0);
    PORTB &= ~(1 << BUZZER_PIN);
    tone_on = 0;
}

/* ============ BUZZER UPDATE ============ */
void buzzer_update(uint16_t signal) {
    if (!buzzer_enabled) {
        buzzer_off();
        return;
    }

    // Ingen signal = ingen lyd
    if (signal == 0) {
        buzzer_off();
        beep_counter = 0;
        return;
    }

    // Beregn beep interval baseret på signal styrke
    // Højere signal = kortere interval = hurtigere beeps
    uint8_t beep_interval;

    if (signal > 150) {
        // Meget stærkt signal: kontinuerlig tone
        buzzer_on();
        return;
    } else if (signal > 100) {
        // Stærkt signal: hurtig beep (hver 2. opdatering ~10Hz)
        beep_interval = 2;
    } else if (signal > 50) {
        // Medium signal: medium beep (hver 4. opdatering ~5Hz)
        beep_interval = 4;
    } else if (signal > 20) {
        // Svagt signal: langsom beep (hver 8. opdatering ~2.5Hz)
        beep_interval = 8;
    } else {
        // Meget svagt signal: meget langsom beep
        beep_interval = 16;
    }

    // Beep logik: tænd i halv interval, sluk i halv interval
    beep_counter++;
    if (beep_counter >= beep_interval) {
        beep_counter = 0;
    }

    // Tone tændt i første halvdel af intervallet
    if (beep_counter < beep_interval / 2) {
        buzzer_on();
    } else {
        buzzer_off();
    }
}

/* ============ BUZZER ENABLE/DISABLE ============ */
void buzzer_enable(uint8_t enabled) {
    buzzer_enabled = enabled;
    if (!enabled) {
        buzzer_off();
    }
}

/* ============ BEKRÆFTELSES BEEP ============ */
void buzzer_beep(uint16_t duration_ms) {
    buzzer_on();
    while (duration_ms > 0) {
        _delay_ms(1);
        duration_ms--;
    }
    buzzer_off();
}
