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
 *   Pin D4 = Knap (skift mellem DFT og Debug skærm)
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

/* ============ MODUL INCLUDES ============ */
#include "config.h"
#include "timer.h"
#include "adc.h"
#include "button.h"
#include "dft.h"
#include "filter.h"
#include "detection.h"
#include "display.h"
#include "buzzer.h"
#include "capture.h"
#include "drivers/I2C.h"
#include "drivers/ssd1306.h"

/* ============ MAIN ============ */
int main(void) {
    // Initialiser hardware - Timer og ADC FØRST (uafhængig af display)
    Timer1_init();
    timer0_init();
    adc_init();
    button_init();
    buzzer_init();
    capture_init(115200);  // UART til MATLAB kommunikation

    // Display init SIDST (kan blokere hvis OLED ikke er tilsluttet)
    I2C_Init();
    InitializeDisplay();
    clear_display();

    // VIGTIGT: Aktiver globale interrupts!
    sei();

    // Vis opstarts besked
    sendStrXY("Starter...", 3, 2);
    _delay_ms(1000);
    clear_display();

    // Knap state til edge detection
    uint8_t btn_debug_prev = 1;
    uint8_t btn_start_prev = 1;
    uint8_t btn_cal_prev = 1;

#if DFT_VERIFICATION_MODE
    uint16_t ready_counter = 0;  // Tæller til periodisk READY signal
    // Send initial READY efter reset
    _delay_ms(100);  // Vent lidt på UART stabilisering
    capture_send_ready();
#endif

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
            if (show_debug)
                display_debug();
            else
                display_dft();
        }

        // Tjek knap D4 - skift mellem DFT og Debug skærm
        uint8_t btn_debug = button_read_debug();
        if (btn_debug_prev && !btn_debug) {     // Falling edge = trykket
            _delay_ms(50);                      // Debounce
            toggle_display_mode();
            clear_display();
        }
        btn_debug_prev = btn_debug;

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

#if DFT_VERIFICATION_MODE
        // MATLAB capture - tjek for seriel kommando
        capture_check_serial();

        // Hvis capture er færdig, send data til MATLAB
        if (capture_get_state() == CAPTURE_DONE) {
            capture_output(Re_buff, Im_buff, mag, ang);
            capture_set_state(CAPTURE_IDLE);
            ready_counter = 0;  // Reset så READY sendes med det samme
        }

        // Send READY periodisk når idle (så MATLAB kan forbinde når som helst)
        if (capture_get_state() == CAPTURE_IDLE) {
            ready_counter++;
            if (ready_counter >= 20) {  // Ca. hvert sekund (20 * 50ms)
                capture_send_ready();
                ready_counter = 0;
            }
        }
#endif
        _delay_ms(50);
    }
}
