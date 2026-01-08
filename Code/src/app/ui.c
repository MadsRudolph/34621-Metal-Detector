/*
 * ui.c
 * Knapper og buzzer for Arduino Nano (ATmega328P)
 *
 * Knapper: Pin 2 (Start/Stop), Pin 4 (Calibrate) - polling med software debounce
 * Buzzer:  Pin 3 (Timer2 PWM via OC2B)
 *
 * ÆNDRING: Fjernet config.h og ATmega2560 kode - kun Nano support.
 *
 * ATmega328P Datasheet Reference (Rev. 7810D–AVR–01/15):
 * ======================================================
 * Timer/Counter2:
 *   - TCCR2A:      Section 17.11.1, Page 127
 *   - TCCR2B:      Section 17.11.2, Page 130
 *   - OCR2B:       Section 17.11.5, Page 131
 *   - Fast PWM:    Section 17.7.3, Page 122
 *   - COM2B bits:  Section 17.11.1, Page 129, Tables 17-5/6/7
 *   - CS2 bits:    Table 17-9, Page 131
 *
 * I/O Ports:
 *   - DDRD:        Section 13.4.9, Page 73
 *   - PORTD:       Section 13.4.8, Page 73
 *   - PIND:        Section 13.4.10, Page 73
 *   - Pull-up:     Section 13.2.1, Page 59
 */

#include "ui.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * PIN OVERSIGT - Arduino Nano (ATmega328P)
 *
 * Knapper (Port D, aktiv lav med pull-up):
 *   Pin 2 (PD2) = Start/Stop
 *   Pin 4 (PD4) = Calibrate
 *
 * Buzzer (Port D, Timer2 PWM):
 *   Pin 3 (PD3) = OC2B
 */

/* Knap events - eksporteret til main.c */
volatile uint8_t btn_start_pressed = 0;
volatile uint8_t btn_calibrate_pressed = 0;

/* Sidste knap tilstande (til edge detection) */
static uint8_t last_start = 1;
static uint8_t last_calibrate = 1;

/*
 * ui_init
 * Konfigurer knapper som input med pull-up
 * Konfigurer Timer2 for buzzer PWM
 */
void ui_init(void)
{
    /*
     * Knapper - input med intern pull-up (aktiv lav)
     * Datasheet: Section 13.2.1, Page 59 - Configuring the Pin
     * Datasheet: Section 13.4.9, Page 73 - DDRD register
     * Datasheet: Section 13.4.8, Page 73 - PORTD register (pull-up)
     */
    DDRD &= ~((1 << PD2) | (1 << PD4));   /* Pin 2 og 4 som input */
    PORTD |= (1 << PD2) | (1 << PD4);     /* Pull-up aktiveret */

    /* Buzzer output - Pin 3 (PD3)
     * Datasheet: Section 13.4.9, Page 73 - DDRD register */
    DDRD |= (1 << PD3);

    /*
     * Timer2 Fast PWM til buzzer
     *
     * FJERNET: #if defined(BUZZER_USE_TIMER4) ... #elif ... kode
     * Nu kun Timer2 kode for Nano.
     *
     * Timer2 Fast PWM mode 3 (TOP = 0xFF)
     * Prescaler 64: 16MHz / 64 / 256 = ~977 Hz
     *
     * Datasheet: Section 17.11.1, Page 127 - TCCR2A register
     * Datasheet: Table 17-8, Page 130 - Waveform Generation Mode
     * Datasheet: Section 17.11.2, Page 130 - TCCR2B register
     * Datasheet: Table 17-9, Page 131 - Clock Select Bit Description
     */
    TCCR2A = (1 << WGM21) | (1 << WGM20);  /* Fast PWM mode 3 */
    TCCR2B = (1 << CS22);                   /* Prescaler 64 */

    /* Start med buzzer slukket
     * Datasheet: Section 17.11.5, Page 131 - OCR2B register */
    OCR2B = 0;
}

/*
 * ui_poll_buttons
 * Kald fra main loop for at tjekke knapper
 * Sætter btn_*_pressed flags ved tryk
 */
void ui_poll_buttons(void)
{
    uint8_t start_now = (PIND & (1 << PD2)) ? 1 : 0;
    uint8_t calibrate_now = (PIND & (1 << PD4)) ? 1 : 0;

    /* Detect falling edge (knap trykket - aktiv lav) */
    if (last_start == 1 && start_now == 0) {
        _delay_ms(50);  /* Debounce 50ms */
        if (!(PIND & (1 << PD2))) {
            btn_start_pressed = 1;
        }
    }

    if (last_calibrate == 1 && calibrate_now == 0) {
        _delay_ms(50);  /* Debounce 50ms */
        if (!(PIND & (1 << PD4))) {
            btn_calibrate_pressed = 1;
        }
    }

    last_start = start_now;
    last_calibrate = calibrate_now;
}

/*
 * buzzer_on
 * Tænd buzzer (50% duty cycle)
 */
void buzzer_on(void)
{
    /*
     * FJERNET: #if defined(BUZZER_USE_TIMER4) Mega kode
     * Nu kun Timer2 for Nano
     *
     * Datasheet: Section 17.11.1, Page 127 - TCCR2A COM2B bits
     * Datasheet: Tables 17-5/6/7, Page 129 - Compare Output Mode (Fast PWM)
     */
    TCCR2A |= (1 << COM2B1);  /* Enable PWM output on OC2B (Pin 3) */
    OCR2B = 127;              /* 50% duty (TOP=255) */
}

/*
 * buzzer_off
 * Sluk buzzer
 *
 * Datasheet: Section 17.11.1, Page 127 - COM2B1:0 = 00 disconnects OC2B
 */
void buzzer_off(void)
{
    TCCR2A &= ~(1 << COM2B1);  /* Disable PWM output */
    OCR2B = 0;
}

/*
 * buzzer_beep
 * Enkelt beep med given varighed
 */
void buzzer_beep(uint16_t duration_ms)
{
    buzzer_on();
    while (duration_ms > 0) {
        _delay_ms(1);
        duration_ms--;
    }
    buzzer_off();
}

/*
 * buzzer_update
 * Opdater buzzer baseret på signal styrke
 * Højere styrke = højere tone og højere volume
 *
 * FJERNET: Al Timer4 (Mega) kode - nu kun Timer2 for Nano
 */
void buzzer_update(uint8_t strength)
{
    if (strength == 0) {
        buzzer_off();
        return;
    }

    /*
     * Timer2 er kun 8-bit, så vi har begrænset frekvens range.
     *
     * Strategi:
     * - Lav styrke (1-50):  Prescaler 64 = ~245 Hz base tone
     * - Høj styrke (51-100): Prescaler 32 = ~490 Hz højere tone
     * - Duty cycle varierer med styrke for perceived loudness
     *
     * Map strength (1-100) til duty (32-224)
     */
    uint8_t duty = 32 + ((uint16_t)strength * 192 / 100);

    if (strength > 50) {
        /* Høj styrke: hurtigere prescaler = højere pitch */
        TCCR2B = (1 << CS21) | (1 << CS20);  /* Prescaler 32 */
    } else {
        /* Lav styrke: langsommere prescaler = lavere pitch */
        TCCR2B = (1 << CS22);  /* Prescaler 64 */
    }

    OCR2B = duty;
    TCCR2A |= (1 << COM2B1);  /* Enable output */
}
