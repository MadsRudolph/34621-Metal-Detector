/*
 * main.c
 * Metal Detektor - Komplet System for Arduino Nano (ATmega328P)
 *
 * State machine med alle komponenter integreret
 *
 * ÆNDRING: Fjernet config.h - alle pin definitioner er nu direkte i filerne.
 *
 * ATmega328P Datasheet Reference (Rev. 7810D–AVR–01/15):
 * ======================================================
 * Interrupts:
 *   - SREG:        Section 6.3.1, Page 10
 *   - sei():       Section 6.7, Page 15 & Section 31, Page 283
 *
 * I/O Ports:
 *   - PIND:        Section 13.4.10, Page 73
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/*
 * PIN OVERSIGT - Arduino Nano (ATmega328P)
 *
 * Knapper (aktiv lav med pull-up):
 *   Pin 2 (PD2) = Start/Stop
 *   Pin 4 (PD4) = Calibrate
 */

#include "signal/tx.h"
#include "signal/dsp.h"
#include "app/display.h"
#include "app/detector.h"
#include "app/ui.h"
#include "app/debug.h"

/* ===== State Machine ===== */
typedef enum {
    STATE_STARTUP,
    STATE_CALIBRATING,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_DEBUG
} system_state_t;

static system_state_t state = STATE_STARTUP;

/* ===== System Init ===== */
static void system_init(void)
{
    /* Signal generation */
    timer0_init();      /* 2kHz TX på Pin 9 (Nano) via Timer0 */

    /* Display */
    display_init();     /* I2C + OLED */

    /* Signal processing */
    adc_init();         /* ADC med Timer1 trigger */

    /* Detection */
    detector_init();    /* Nulstil kalibrering */

    /* User interface */
    ui_init();          /* Knapper + buzzer */

    /* Debug system */
    debug_init();       /* Debug skærme og pin */
}

/* ===== Calibration Routine ===== */
static void run_calibration(void)
{
    screen_calibrating();

    /* Saml data i 2 sekunder */
    for (uint16_t i = 0; i < 200; i++) {
        if (DFT_done) {
            DFT_done = 0;
            DFT_Calc();
        }
        _delay_ms(10);
    }

    detector_calibrate();

    /* Bekræft med dobbelt beep */
    buzzer_beep(50);
    _delay_ms(50);
    buzzer_beep(50);
}

/* ===== Main ===== */
int main(void)
{
    /* Initialiser alle subsystemer */
    system_init();

    /*
     * TILFØJET: Aktiver globale interrupts
     *
     * KRITISK FEJL I ORIGINAL KODE:
     * sei() manglede helt! Uden sei() er globale interrupts disabled,
     * og Timer0 COMPA ISR + ADC ISR vil ALDRIG blive kaldt.
     * Resultatet er at TX signal ikke genereres og ADC ikke sampler.
     *
     * Datasheet: Section 6.3.1, Page 10 - SREG I-bit (Global Interrupt Enable)
     * Datasheet: Section 6.7, Page 15 - Reset and Interrupt Handling
     */
    sei();

    /* Splash screen */
    screen_splash();
    buzzer_beep(100);
    _delay_ms(1500);

    /* Start sampling */
    sampling_start();

    /* Initial kalibrering */
    state = STATE_CALIBRATING;
    run_calibration();

    /* Start i idle */
    state = STATE_IDLE;
    screen_idle();

    /* Hovedloop */
    while (1) {
        /* Poll knapper */
        ui_poll_buttons();

        /*
         * Check for debug mode entry: HOLD begge knapper i IDLE
         * Læser pin status direkte (aktiv lav med pull-up)
         */
        if (state == STATE_IDLE) {
            uint8_t start_held = !(PIND & (1 << PD2));
            uint8_t calib_held = !(PIND & (1 << PD4));

            if (start_held && calib_held) {
                /* Vent lidt og tjek igen (debounce) */
                _delay_ms(100);
                start_held = !(PIND & (1 << PD2));
                calib_held = !(PIND & (1 << PD4));

                if (start_held && calib_held) {
                    /* Begge holdes stadig - gå i debug mode */
                    btn_start_pressed = 0;
                    btn_calibrate_pressed = 0;
                    debug_toggle();
                    state = STATE_DEBUG;
                    buzzer_beep(50);
                    _delay_ms(50);
                    buzzer_beep(50);

                    /* Vent til knapper slippes */
                    while (!(PIND & (1 << PD2)) || !(PIND & (1 << PD4))) {
                        _delay_ms(10);
                    }
                }
            }
        }

        /* State machine */
        switch (state) {

            case STATE_IDLE:
                /* Vent på Start knap */
                if (btn_start_pressed) {
                    btn_start_pressed = 0;
                    state = STATE_RUNNING;
                    display_clear();
                    buzzer_beep(50);
                }
                /* Kalibrer knap */
                if (btn_calibrate_pressed) {
                    btn_calibrate_pressed = 0;
                    state = STATE_CALIBRATING;
                    run_calibration();
                    state = STATE_IDLE;
                    screen_idle();
                }
                break;

            case STATE_RUNNING:
                /* Process DFT og opdater display */
                if (DFT_done) {
                    DFT_done = 0;
                    DFT_Calc();

                    metal_type_t metal = detector_classify();
                    uint8_t strength = detector_get_strength();

                    /* Opdater display */
                    screen_detection(strength, phase_filt, (uint8_t)metal);

                    /* Opdater buzzer */
                    buzzer_update(strength);
                }

                /* Stop knap */
                if (btn_start_pressed) {
                    btn_start_pressed = 0;
                    state = STATE_IDLE;
                    buzzer_off();
                    screen_idle();
                    buzzer_beep(100);
                }
                /* Kalibrer knap */
                if (btn_calibrate_pressed) {
                    btn_calibrate_pressed = 0;
                    buzzer_off();
                    state = STATE_CALIBRATING;
                    run_calibration();
                    state = STATE_RUNNING;
                    display_clear();
                }
                break;

            case STATE_CALIBRATING:
                /* Håndteres af run_calibration() */
                break;

            case STATE_STARTUP:
                /* Håndteres før loop */
                break;

            case STATE_DEBUG:
                /* Opdater debug display */
                debug_update();

                /* Start knap = skift debug skærm */
                if (btn_start_pressed) {
                    btn_start_pressed = 0;
                    debug_next_screen();
                }

                /* Kalibrer knap = afslut debug mode */
                if (btn_calibrate_pressed) {
                    btn_calibrate_pressed = 0;
                    debug_toggle();  /* Slå debug fra */
                    state = STATE_IDLE;
                    screen_idle();
                    buzzer_beep(100);
                }
                break;
        }

        /* Rate limiting (~20Hz loop) */
        _delay_ms(50);
    }

    return 0;
}
