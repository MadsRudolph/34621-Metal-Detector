#include "tx.h"
#include <avr/io.h>

void timer1_init(void) {
    DDRB |= (1 << PB5);   /* Pin 11 = OC1A */

    /* CTC mode, toggle OC1A on compare match */
    TCCR1A = (1 << COM1A0);              /* Toggle OC1A on match */
    TCCR1B = (1 << WGM12) | (1 << CS10); /* CTC mode, no prescaler */

    OCR1A = 3999;  /* 16 MHz / 4000 = 4 kHz toggle = 2 kHz square wave */
}