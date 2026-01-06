/**
 * uart.c
 * UART driver implementation for ATmega2560
 *
 * Registers used:
 * - UBRR0H/L: Baud rate register
 * - UCSR0A: Status register (UDRE0 = data register empty)
 * - UCSR0B: Control register (TXEN0 = transmit enable)
 * - UCSR0C: Frame format (8N1)
 * - UDR0: Data register
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#include "uart.h"
#include "config.h"
#include <avr/io.h>

void uart_init(void)
{
    /* Set baud rate: UBRR = F_CPU/(16*baud) - 1 = 103 for 9600 @ 16MHz */
    UBRR0H = (uint8_t)(UART_UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)(UART_UBRR_VALUE & 0xFF);

    /* Enable transmitter only (no RX needed for power test) */
    UCSR0B = (1 << TXEN0);

    /* Frame format: 8 data bits, no parity, 1 stop bit (8N1) */
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_putchar(char c)
{
    /* Wait for empty transmit buffer (UDRE0 = 1) */
    while (!(UCSR0A & (1 << UDRE0)))
        ;

    /* Write data to transmit buffer */
    UDR0 = c;
}

void uart_puts(const char *s)
{
    while (*s) {
        /* Convert \n to \r\n for terminal compatibility */
        if (*s == '\n') {
            uart_putchar('\r');
        }
        uart_putchar(*s++);
    }
}

void uart_print_int(int32_t value)
{
    char buffer[12];  /* -2147483648 + null = 12 chars max */
    char *ptr = buffer + sizeof(buffer) - 1;
    uint8_t negative = 0;

    *ptr = '\0';

    if (value == 0) {
        uart_putchar('0');
        return;
    }

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    while (value > 0) {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }

    if (negative) {
        *--ptr = '-';
    }

    uart_puts(ptr);
}

void uart_print_uint(uint32_t value)
{
    char buffer[11];  /* 4294967295 + null = 11 chars max */
    char *ptr = buffer + sizeof(buffer) - 1;

    *ptr = '\0';

    if (value == 0) {
        uart_putchar('0');
        return;
    }

    while (value > 0) {
        *--ptr = '0' + (value % 10);
        value /= 10;
    }

    uart_puts(ptr);
}

void uart_print_fixed1(int32_t value_x10)
{
    uint8_t negative = 0;

    if (value_x10 < 0) {
        negative = 1;
        value_x10 = -value_x10;
    }

    int32_t integer_part = value_x10 / 10;
    int32_t decimal_part = value_x10 % 10;

    if (negative) {
        uart_putchar('-');
    }

    uart_print_int(integer_part);
    uart_putchar('.');
    uart_putchar('0' + decimal_part);
}
