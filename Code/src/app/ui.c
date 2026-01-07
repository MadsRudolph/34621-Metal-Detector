/*
 * ui.c
 * Knapper og buzzer
 *
 * Knapper bruger polling med software debounce
 * Buzzer bruger Timer4 PWM på Pin 8 (OC4C)
 */

#include "ui.h"
#include <avr/io.h>
#include <util/delay.h>

/* Debounce tid i ms */
#define DEBOUNCE_MS 50

/* Buzzer frekvens (~2kHz) */
#define BUZZER_TOP 3999

/* Knap events */
volatile uint8_t btn_start_pressed = 0;
volatile uint8_t btn_calibrate_pressed = 0;

/* Sidste knap tilstande (til edge detection) */
static uint8_t last_start = 1;
static uint8_t last_calibrate = 1;

/*
 * ui_init
 * Konfigurer knapper som input med pull-up
 * Konfigurer Timer4 for buzzer PWM
 */
void ui_init(void)
{
    /*
     * Knapper på PE4 (Pin 2) og PE5 (Pin 3)
     * Input med intern pull-up (aktiv lav)
     */
    DDRE &= ~((1 << PE4) | (1 << PE5));   /* Input */
    PORTE |= (1 << PE4) | (1 << PE5);     /* Pull-up */

    /*
     * Buzzer på PH5 (Pin 8) - Timer4 OC4C
     * Fast PWM, TOP = ICR4
     */
    DDRH |= (1 << PH5);  /* Output */

    /* Timer4: Fast PWM, TOP = ICR4 */
    TCCR4A = (1 << WGM41);
    TCCR4B = (1 << WGM43) | (1 << WGM42) | (1 << CS41);  /* Prescaler 8 */

    /* Sæt frekvens: 16MHz / 8 / 4000 = 500Hz */
    ICR4 = BUZZER_TOP;

    /* Start med buzzer slukket */
    OCR4C = 0;
}

/*
 * ui_poll_buttons
 * Kald fra main loop for at tjekke knapper
 * Sætter btn_*_pressed flags ved tryk
 */
void ui_poll_buttons(void)
{
    uint8_t start_now = (PINE & (1 << PE4)) ? 1 : 0;
    uint8_t calibrate_now = (PINE & (1 << PE5)) ? 1 : 0;

    /* Detect falling edge (knap trykket) */
    if (last_start == 1 && start_now == 0) {
        _delay_ms(DEBOUNCE_MS);
        if (!(PINE & (1 << PE4))) {
            btn_start_pressed = 1;
        }
    }

    if (last_calibrate == 1 && calibrate_now == 0) {
        _delay_ms(DEBOUNCE_MS);
        if (!(PINE & (1 << PE5))) {
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
    TCCR4A |= (1 << COM4C1);  /* Enable PWM output */
    OCR4C = BUZZER_TOP / 2;   /* 50% duty */
}

/*
 * buzzer_off
 * Sluk buzzer
 */
void buzzer_off(void)
{
    TCCR4A &= ~(1 << COM4C1);  /* Disable PWM output */
    OCR4C = 0;
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

    /*
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
}
