/*
 * tx.c
 * TX signal generation using Timer1
 *
 * Generates 2 kHz square wave on Timer1 OC1A output:
 *   - ATmega2560: Pin 11 (PB5)
 *   - ATmega328P: Pin 9 (PB1)
 */

#include "tx.h"
#include "../config.h"
#include <avr/io.h>

void timer1_init(void) {
    /* Set TX PWM pin as output */
    TX_PWM_DDR |= (1 << TX_PWM_BIT);

    /* CTC mode, toggle OC1A on compare match */
    TCCR1A = (1 << COM1A0);              /* Toggle OC1A on match */
    TCCR1B = (1 << WGM12) | (1 << CS10); /* CTC mode, no prescaler */

    OCR1A = TX_TIMER1_TOP;  /* 16 MHz / 4000 = 4 kHz toggle = 2 kHz square wave */
}
