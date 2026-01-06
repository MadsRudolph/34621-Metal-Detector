/**
 * uart.h
 * UART driver for ATmega2560 (UART0)
 *
 * Hardware: UART0 on PE0(RX)/PE1(TX) = Arduino Pins 0/1
 * Baud: 9600, 8N1
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>

/**
 * Initialize UART0 for 9600 baud, 8N1
 */
void uart_init(void);

/**
 * Transmit a single character (blocking)
 */
void uart_putchar(char c);

/**
 * Transmit a null-terminated string
 * Converts \n to \r\n for terminal compatibility
 */
void uart_puts(const char *s);

/**
 * Print a signed integer in decimal
 */
void uart_print_int(int32_t value);

/**
 * Print an unsigned integer in decimal
 */
void uart_print_uint(uint32_t value);

/**
 * Print a fixed-point value with 1 decimal place
 * value is multiplied by 10 (e.g., 123 prints as "12.3")
 */
void uart_print_fixed1(int32_t value_x10);

#endif /* UART_H */
