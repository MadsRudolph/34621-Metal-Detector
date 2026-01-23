/*
 * Metal Detektor - Hovedprogram
 *
 * Hardware:
 *   Pin 9  = TX signal (2kHz firkantbølge til sendespole)
 *   Pin 11 = Buzzer output (PWM tone)
 *   Pin A0 = RX signal (modtaget signal fra modtagerspole)
 *   Pin A4 = I2C SDA (OLED display)
 *   Pin A5 = I2C SCL (OLED display)
 *   Pin D2 = Knap (Start/Stop detektor - Krav 9a)
 *   Pin D3 = Knap (Kalibrering - Krav 9b)
 *
 * Hvordan det virker:
 *   1. Timer0 genererer 8kHz interrupt og toggler TX pin hver 2. gang = 2kHz
 *   2. ADC sampler automatisk ved hver Timer0 interrupt = 8kHz sampling
 *   3. DFT akkumulerer 64 samples og beregner magnitude og fase
 *   4. Resultater vises på OLED display
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* --- Modul includes --- */
#include "include/config.h"
#include "include/timer.h"
#include "include/adc.h"
#include "include/button.h"
#include "include/dft.h"
#include "include/filter.h"
#include "include/detection.h"
#include "include/display.h"
#include "include/buzzer.h"
#include "include/jingle.h"
#include "drivers/I2C.h"
#include "drivers/ssd1306.h"

/* --- Main --- */
int main(void) {
    // Initialiser hardware - Timer og ADC FØRST (uafhængig af display)
    Timer1_init();
    timer0_init();
    adc_init();
    button_init();
    buzzer_init();

    // Display init SIDST (kan blokere hvis OLED ikke er tilsluttet)
    I2C_Init();
    InitializeDisplay();
    clear_display();

    // VIGTIGT: Aktiver globale interrupts!
    sei();

    // Splash skærm + opstartslyd
    display_splash();
    play_startup_jingle();
    _delay_ms(500);
    clear_display();

    // Knap state til edge detection
    uint8_t btn_start_prev = 1;
    uint8_t btn_cal_prev = 1;

    // Hovedløkke
    while (1) {
        // Sleep mode styring
        if (do_sleep) {
            SMCR |= (1 << SE);      // Enable sleep mode (Idle mode)
        } else {
            SMCR &= ~(1 << SE);     // Disable sleep mode
        }

        // Når DFT vindue er færdigt, beregn og vis resultater
        if (DFT_done) {
            DFT_done = 0;
            if (detection_active) {
                DFT_Calc();         // Beregn magnitude og fase
                IIR_Filt();         // Filtrer værdier

                // Opdater buzzer baseret på signal styrke
                uint16_t signal = 0;
                if (is_calibrated && mag_filtered > cal_mag) {
                    signal = mag_filtered - cal_mag;
                }
                buzzer_update(signal);
            } else {
                // Detektor pauset - sluk buzzer
                buzzer_update(0);
            }

            // Opdater display
            display_dft();
        }

        // Tjek knap D2 - Start/Stop detektor (Krav 9a)
        uint8_t btn_start = button_read_start_stop();
        if (btn_start_prev && !btn_start) {    // Falling edge = trykket
            _delay_ms(50);                      // Debounce
            toggle_detection();
            clear_display();
        }
        btn_start_prev = btn_start;

        // Tjek knap D3 - Kalibrering (Krav 9b)
        uint8_t btn_cal = button_read_calibrate();
        if (btn_cal_prev && !btn_cal) {        // Falling edge = trykket
            _delay_ms(50);                      // Debounce
            calibrate();
            buzzer_beep(100);                   // Bekræftelses-beep
            display_calibrated();
            _delay_ms(1000);
            clear_display();
        }
        btn_cal_prev = btn_cal;

        _delay_ms(50);
    }
}
