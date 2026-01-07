/**
 * main.c - Prime Number Calculator Test
 * Simple test program that calculates prime numbers and displays them on OLED
 */

#include "config.h"
#include "I2C.h"
#include "ssd1306.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

/* Check if a number is prime */
static uint8_t is_prime(uint16_t n)
{
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    for (uint16_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

/* Convert number to string */
static void uint_to_str(uint16_t n, char *buf)
{
    char tmp[8];
    uint8_t i = 0;

    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    while (n > 0) {
        tmp[i++] = '0' + (n % 10);
        n /= 10;
    }

    /* Reverse */
    uint8_t j = 0;
    while (i > 0) {
        buf[j++] = tmp[--i];
    }
    buf[j] = '\0';
}

int main(void)
{
    /* Initialize I2C and OLED display */
    I2C_Init();
    InitializeDisplay();
    clear_display();

    /* Show title */
    sendStrXY("Prime Numbers", 0, 0);
    sendStrXY("Calculator", 1, 0);
    _delay_ms(2000);

    uint16_t current_num = 2;
    uint16_t prime_count = 0;
    char num_str[8];
    char count_str[8];
    char line_buf[22];

    while (1) {
        /* Find next prime */
        while (!is_prime(current_num)) {
            current_num++;
        }

        prime_count++;

        /* Clear and display results */
        clear_display();

        /* Line 0: Title */
        sendStrXY("=== PRIMES ===", 0, 0);

        /* Line 2: Current prime */
        uint_to_str(current_num, num_str);
        strcpy(line_buf, "Prime: ");
        strcat(line_buf, num_str);
        sendStrXY(line_buf, 2, 0);

        /* Line 4: Count */
        uint_to_str(prime_count, count_str);
        strcpy(line_buf, "Count: ");
        strcat(line_buf, count_str);
        sendStrXY(line_buf, 4, 0);

        /* Line 6: Show some calculation activity */
        uint_to_str(current_num * current_num, num_str);
        strcpy(line_buf, "Square: ");
        strcat(line_buf, num_str);
        sendStrXY(line_buf, 6, 0);

        /* Move to next number */
        current_num++;

        /* Delay so we can see the display */
        _delay_ms(500);
    }

    return 0;
}
