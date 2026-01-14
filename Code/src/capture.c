/*
 * DFT Capture Mode - MATLAB Verification
 * Captures raw ADC samples and outputs via UART for verification
 */

#include "capture.h"

#if DFT_VERIFICATION_MODE

#include <avr/io.h>
#include <stdio.h>

static volatile int16_t samples[N_SAMPLES];
static volatile uint8_t capture_state = CAPTURE_IDLE;


/* ============ UART ============ */

void capture_init(uint32_t baud) {
    UCSR0A = (1 << U2X0);
    uint16_t ubrr = F_CPU / 8 / baud - 1;
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static uint8_t uart_available(void) { return UCSR0A & (1 << RXC0); }
static char uart_getchar(void) { return UDR0; }
static void uart_putchar(char c) { while (!(UCSR0A & (1 << UDRE0))); UDR0 = c; }
static void uart_puts(const char *s) { while (*s) uart_putchar(*s++); }


/* ============ STATE ============ */

uint8_t capture_get_state(void) { return capture_state; }
void capture_set_state(uint8_t state) { capture_state = state; }


/* ============ SAMPLE STORAGE ============ */

void capture_store_sample(uint8_t index, int16_t sample) {
    if (capture_state == CAPTURE_ACTIVE) {
        samples[index] = sample;
    }
}


/* ============ SERIAL INTERFACE ============ */

void capture_send_ready(void) {
    uart_puts("READY\r\n");
}

void capture_check_serial(void) {
    if (uart_available()) {
        char c = uart_getchar();
        if ((c == 'C' || c == 'c') && capture_state == CAPTURE_IDLE) {
            capture_state = CAPTURE_WAITING;
            uart_puts("ACK\r\n");
        }
    }
}

void capture_output(int32_t re, int32_t im, uint16_t mag, int16_t phase) {
    char buf[24];

    uart_puts("\r\n--- Sample Capture ---\r\n");
    uart_puts("sample,value\r\n");
    for (uint8_t i = 0; i < N_SAMPLES; i++) {
        sprintf(buf, "%d,%d\r\n", i, samples[i]);
        uart_puts(buf);
    }

    uart_puts("\r\n--- Arduino Results ---\r\n");
    sprintf(buf, "Re,%ld\r\n", re);     uart_puts(buf);
    sprintf(buf, "Im,%ld\r\n", im);     uart_puts(buf);
    sprintf(buf, "Mag,%u\r\n", mag);    uart_puts(buf);
    sprintf(buf, "Phase,%d\r\n", phase); uart_puts(buf);
    uart_puts("--- End Capture ---\r\n\r\n");
}

#endif /* DFT_VERIFICATION_MODE */
