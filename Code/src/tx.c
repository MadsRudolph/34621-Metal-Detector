/**
 * tx.c
 * TX Signal Generator implementation
 *
 * Timer1 Configuration (ATmega2560):
 * - Mode: CTC (WGM12=1, WGM11=0, WGM10=0)
 * - Output: Toggle OC1A on compare match (COM1A0=1)
 * - Prescaler: 1 (CS10=1)
 * - OCR1A: 3999 for 2kHz output
 *
 * Calculation:
 * f_out = F_CPU / (2 * prescaler * (1 + OCR1A))
 * f_out = 16MHz / (2 * 1 * 4000) = 2000 Hz
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#include "tx.h"
#include "config.h"
#include <avr/io.h>

void tx_init(void)
{
    /* Configure OC1A (PB5) as output */
    TX_DDR |= (1 << TX_PIN);

    /* Clear timer registers */
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    /* Set compare value for 2kHz output */
    OCR1A = TX_OCR1A_VALUE;

    /*
     * TCCR1A: COM1A0 = toggle OC1A on compare match
     * TCCR1B: WGM12 = CTC mode, CS10 = no prescaler
     */
}

void tx_start(void)
{
    /* Enable toggle on compare match (COM1A0=1) */
    TCCR1A = (1 << COM1A0);

    /* CTC mode (WGM12=1), prescaler /1 (CS10=1) */
    TCCR1B = (1 << WGM12) | (1 << CS10);
}

void tx_stop(void)
{
    /* Disable timer */
    TCCR1A = 0;
    TCCR1B = 0;

    /* Set output pin LOW */
    TX_PORT &= ~(1 << TX_PIN);
}
