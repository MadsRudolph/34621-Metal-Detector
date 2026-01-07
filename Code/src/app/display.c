/*
 * display.c
 * Simple wrapper for OLED display
 */

#include "display.h"
#include "../drivers/I2C.h"
#include "../drivers/ssd1306.h"

void display_init(void)
{
    I2C_Init();           /* Start I2C bus */
    InitializeDisplay();  /* Configure OLED */
    clear_display();      /* Start with blank screen */
}

void display_clear(void)
{
    clear_display();
}

void display_text(uint8_t row, uint8_t col, const char *text)
{
    /* sendStrXY expects: (string, row, col) */
    sendStrXY((char *)text, row, col);
}

void display_number(uint8_t row, uint8_t col, uint16_t value)
{
    char buf[6];  /* Max "65535" + null */
    char *p = buf + 5;

    *p = '\0';

    /* Convert digits right-to-left */
    do {
        *(--p) = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    display_text(row, col, p);
}
