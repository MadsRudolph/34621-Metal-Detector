/*
 * ui.c
 * Knapper og buzzer
 *
 * Knapper bruger polling med software debounce
 * Buzzer bruger:
 *   - ATmega2560: Timer4 PWM on Pin 8 (OC4C)
 *   - ATmega328P: Timer2 PWM on Pin 3 (OC2B)
 */

#include "ui.h"
#include "../config.h"
#include <avr/io.h>
#include <util/delay.h>

/* Knap events */
volatile uint8_t btn_start_pressed = 0;
volatile uint8_t btn_calibrate_pressed = 0;

/* Sidste knap tilstande (til edge detection) */
static uint8_t last_start = 1;
static uint8_t last_calibrate = 1;

/*
 * ui_init
 * Konfigurer knapper som input med pull-up
 * Konfigurer Timer for buzzer PWM
 */
void ui_init(void)
{
    /*
     * Knapper - input med intern pull-up (aktiv lav)
     * Uses config.h definitions for portability
     */
    BTN_DDR &= ~((1 << BTN_START_BIT) | (1 << BTN_CALIB_BIT));   /* Input */
    BTN_PORT |= (1 << BTN_START_BIT) | (1 << BTN_CALIB_BIT);     /* Pull-up */

    /*
     * Buzzer output
     */
    BUZZER_DDR |= (1 << BUZZER_BIT);

#if defined(BUZZER_USE_TIMER4)
    /*
     * ATmega2560: Timer4 Fast PWM, TOP = ICR4
     * Pin 8 (PH5/OC4C)
     */
    TCCR4A = (1 << WGM41);
    TCCR4B = (1 << WGM43) | (1 << WGM42) | (1 << CS41);  /* Prescaler 8 */

    /* Sæt frekvens: 16MHz / 8 / 4000 = 500Hz */
    ICR4 = 3999;

    /* Start med buzzer slukket */
    OCR4C = 0;

#elif defined(BUZZER_USE_TIMER2)
    /*
     * ATmega328P: Timer2 Fast PWM, TOP = OCR2A (mode 7)
     * Pin 3 (PD3/OC2B)
     *
     * For variable frequency, we use mode 7 (Fast PWM with OCR2A as TOP)
     * This limits resolution but allows frequency control
     */
    TCCR2A = (1 << WGM21) | (1 << WGM20);  /* Fast PWM mode 3 (TOP=0xFF) */
    TCCR2B = (1 << CS22);                   /* Prescaler 64: 16MHz/64 = 250kHz */

    /* Start med buzzer slukket */
    OCR2B = 0;
#endif
}

/*
 * ui_poll_buttons
 * Kald fra main loop for at tjekke knapper
 * Sætter btn_*_pressed flags ved tryk
 */
void ui_poll_buttons(void)
{
    uint8_t start_now = (BTN_PIN & (1 << BTN_START_BIT)) ? 1 : 0;
    uint8_t calibrate_now = (BTN_PIN & (1 << BTN_CALIB_BIT)) ? 1 : 0;

    /* Detect falling edge (knap trykket) */
    if (last_start == 1 && start_now == 0) {
        _delay_ms(DEBOUNCE_MS);
        if (!(BTN_PIN & (1 << BTN_START_BIT))) {
            btn_start_pressed = 1;
        }
    }

    if (last_calibrate == 1 && calibrate_now == 0) {
        _delay_ms(DEBOUNCE_MS);
        if (!(BTN_PIN & (1 << BTN_CALIB_BIT))) {
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
#if defined(BUZZER_USE_TIMER4)
    TCCR4A |= (1 << COM4C1);  /* Enable PWM output */
    OCR4C = 3999 / 2;         /* 50% duty */

#elif defined(BUZZER_USE_TIMER2)
    TCCR2A |= (1 << COM2B1);  /* Enable PWM output on OC2B */
    OCR2B = 127;              /* 50% duty (TOP=255) */
#endif
}

/*
 * buzzer_off
 * Sluk buzzer
 */
void buzzer_off(void)
{
#if defined(BUZZER_USE_TIMER4)
    TCCR4A &= ~(1 << COM4C1);  /* Disable PWM output */
    OCR4C = 0;

#elif defined(BUZZER_USE_TIMER2)
    TCCR2A &= ~(1 << COM2B1);  /* Disable PWM output */
    OCR2B = 0;
#endif
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
 * Opdater buzzer frekvens baseret på signal styrke
 * Højere styrke = højere tone (humming)
 */
void buzzer_update(uint8_t strength)
{
    if (strength == 0) {
        buzzer_off();
        return;
    }

#if defined(BUZZER_USE_TIMER4)
    /*
     * ATmega2560: Full 16-bit timer range
     * Frekvens baseret på styrke:
     * styrke 1   = lav tone  (~200 Hz, TOP = 10000)
     * styrke 100 = høj tone  (~2000 Hz, TOP = 1000)
     *
     * TOP = 10000 - (strength * 90)
     */
    uint16_t top = 10000 - ((uint16_t)strength * 90);

    ICR4 = top;
    OCR4C = top / 2;  /* 50% duty */
    TCCR4A |= (1 << COM4C1);  /* Enable output */

#elif defined(BUZZER_USE_TIMER2)
    /*
     * ATmega328P: 8-bit timer, limited frequency range
     * With prescaler 64 and TOP=255: freq = 250kHz/256 = ~977 Hz
     * With prescaler 256 and TOP=255: freq = 62.5kHz/256 = ~244 Hz
     *
     * We vary duty cycle to create perceived loudness change
     * and use prescaler switching for frequency range
     *
     * Low strength = low duty, high strength = high duty
     * Map strength (1-100) to duty (32-224)
     */
    uint8_t duty = 32 + ((uint16_t)strength * 192 / 100);

    /* Higher strength = faster prescaler = higher pitch */
    if (strength > 50) {
        /* Prescaler 32: ~490 Hz base */
        TCCR2B = (1 << CS21) | (1 << CS20);  /* Prescaler 32 */
    } else {
        /* Prescaler 64: ~245 Hz base */
        TCCR2B = (1 << CS22);  /* Prescaler 64 */
    }

    OCR2B = duty;
    TCCR2A |= (1 << COM2B1);  /* Enable output */
#endif
}
