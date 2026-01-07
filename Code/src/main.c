/*
 * main.c
 * Metal Detector - Main Entry Point
 *
 * Phase 4: Test 8kHz sampling with 64-sample buffer
 */

#include <avr/io.h>
#include <util/delay.h>

#include "signal/tx.h"
#include "signal/rx.h"
#include "app/display.h"

int main(void)
{
    uint16_t buffer_count = 0;

    /* Start 2kHz TX signal (Pin 11) */
    timer1_init();

    /* Initialize OLED display */
    display_init();

    /* Initialize ADC */
    adc_init();

    /* Show startup info */
    display_text(0, 0, "8kHz Sampling");
    display_text(2, 0, "Buffers:");
    display_text(4, 0, "Last[0]:");
    display_text(5, 0, "Last[31]:");
    display_text(6, 0, "Last[63]:");

    /* Start 8kHz sampling */
    sampling_start();

    while (1) {
        /* Check if buffer is ready */
        if (buffer_ready) {
            buffer_count++;

            /* Display buffer count */
            display_text(2, 9, "      ");
            display_number(2, 9, buffer_count);

            /* Display some sample values */
            display_text(4, 9, "     ");
            display_number(4, 9, adc_buffer[0]);

            display_text(5, 9, "     ");
            display_number(5, 9, adc_buffer[31]);

            display_text(6, 9, "     ");
            display_number(6, 9, adc_buffer[63]);

            /* Clear flag - allows next buffer to fill */
            buffer_ready = 0;

            /* Slow down display updates */
            _delay_ms(100);
        }
    }

    return 0;
}
