/**
 * main.c - Power Test with debug
 */

#include "config.h"
#include "uart.h"
#include "tx.h"
#include "sampling.h"
#include "dsp.h"
#include "I2C.h"
#include "ssd1306.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

static volatile uint32_t millis_counter = 0;

static void timer0_init(void)
{
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

ISR(TIMER0_COMPA_vect) { millis_counter++; }

static uint32_t millis(void)
{
    uint32_t ms;
    cli();
    ms = millis_counter;
    sei();
    return ms;
}

static void power_save_init(void)
{
    PRR0 |= (1 << PRUSART1);
    PRR1 |= (1 << PRUSART2) | (1 << PRUSART3);
    PRR1 |= (1 << PRTIM4) | (1 << PRTIM5);
    PRR0 |= (1 << PRSPI);
}

static volatile uint32_t dft_count = 0;
static volatile uint32_t oled_count = 0;
static volatile uint32_t last_stats_time = 0;

static void display_update(float magnitude, float phase_deg)
{
    uint8_t mag_cols = (uint8_t)(magnitude * 16.0f / 100.0f);
    if (mag_cols > 16) mag_cols = 16;
    uint8_t phase_cols = (uint8_t)(phase_deg * 16.0f / 360.0f);
    if (phase_cols > 16) phase_cols = 16;

    setXY(0, 0);
    for (uint8_t col = 0; col < 16; col++) {
        for (uint8_t i = 0; i < 8; i++) {
            SendChar(col < mag_cols ? 0xFF : 0x00);
        }
    }
    setXY(2, 0);
    for (uint8_t col = 0; col < 16; col++) {
        for (uint8_t i = 0; i < 8; i++) {
            SendChar(col < phase_cols ? 0xFF : 0x00);
        }
    }
}

int main(void)
{
    uart_init();
    uart_puts("\n\n=== Power Test Debug ===\n\n");

    power_save_init();
    adc_init();
    
    uart_puts("ADC regs: ADMUX=");
    uart_print_uint(ADMUX);
    uart_puts(" ADCSRA=");
    uart_print_uint(ADCSRA);
    uart_puts("\n");

    I2C_Init();
    InitializeDisplay();
    clear_display();

    uart_puts("After OLED: ADMUX=");
    uart_print_uint(ADMUX);
    uart_puts(" ADCSRA=");
    uart_print_uint(ADCSRA);
    uart_puts("\n");

    timer0_init();
    tx_init();
    sampling_timer_init();
    dsp_init();
    sei();

    tx_start();
    sampling_start();

    uart_puts("Running. Checking first buffer...\n");
    _delay_ms(100);

    if (sampling_buffer_ready()) {
        volatile uint16_t *buf = sampling_get_buffer();
        uart_puts("Buffer[0-7]: ");
        for (uint8_t i = 0; i < 8; i++) {
            uart_print_uint(buf[i]);
            uart_puts(" ");
        }
        uart_puts("\n");
        
        /* Process and show result */
        dsp_process(buf, SAMPLE_BUFFER_SIZE);
        const dsp_result_t *r = dsp_get_result();
        uart_puts("DSP result: Mag=");
        uart_print_fixed1((int32_t)(r->magnitude * 10.0f));
        uart_puts(" Phase=");
        uart_print_fixed1((int32_t)(r->phase_deg * 10.0f));
        uart_puts("\n\n");
    } else {
        uart_puts("ERROR: No buffer!\n");
    }

    uint32_t last_oled_update = millis();
    last_stats_time = millis();

    while (1) {
        uint32_t now = millis();

        if (sampling_buffer_ready()) {
            volatile uint16_t *buffer = sampling_get_buffer();
            
            /* Debug: show samples once per second */
            if (dft_count == 0) {
                uart_puts("[");
                for (uint8_t i = 0; i < 4; i++) {
                    uart_print_uint(buffer[i]);
                    uart_puts(" ");
                }
                uart_puts("]\n");
            }
            
            dsp_process(buffer, SAMPLE_BUFFER_SIZE);
            dft_count++;
        }

        if (now - last_oled_update >= OLED_UPDATE_MS) {
            last_oled_update = now;
            const dsp_result_t *result = dsp_get_result();
            display_update(result->magnitude, result->phase_deg);
            oled_count++;
        }

        if (now - last_stats_time >= SERIAL_UPDATE_MS) {
            uint32_t elapsed = now - last_stats_time;
            uint32_t dft_rate = (dft_count * 1000) / elapsed;
            uint32_t oled_fps = (oled_count * 1000) / elapsed;
            const dsp_result_t *result = dsp_get_result();

            uart_puts("Mag:");
            uart_print_fixed1((int32_t)(result->magnitude * 10.0f));
            uart_puts(" Ph:");
            uart_print_fixed1((int32_t)(result->phase_deg * 10.0f));
            uart_puts(" DFT:");
            uart_print_uint(dft_rate);
            uart_puts(" OLED:");
            uart_print_uint(oled_fps);
            uart_puts("\n");

            dft_count = 0;
            oled_count = 0;
            last_stats_time = now;
        }
    }
    return 0;
}
