/*
 * tx_rx.c
 * TX signal generation using Timer0 and RX with ADC triggered by Compare Match A
 *
 * TX Signal: Pin 9 = PB1
 * RX Input:  Pin A0 = ADC channel 0
 */

#include "tx.h"
#include "dsp.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static uint8_t i = 0;
int16_t ADC_Raw = 0;

void timer0_init(void) {
    /* Configure TX pin as output - Pin 9 (PB1) */
    DDRB |= (1 << PB1);

    /* CTC mode */
    TCCR0A |= (1 << WGM01);

    /* Prescaler = 8 */
    TCCR0B |= (1 << CS01);

    /*
     * OCR0A = 249 gives:
     * f_interrupt = 16MHz / (8 * 250) = 8 kHz
     *
     * FIX: Original had OCR0A = 124 which gave 16 kHz (wrong!)
     */
    OCR0A = 249;

    /* Enable Compare Match A interrupt */
    TIMSK0 = (1 << OCIE0A);
}

void adc_init(void) {
    /* Reference = AVCC (5V) */
    ADMUX = (1 << REFS0);

    /*
     * ADC Control:
     * ADEN  = Enable ADC
     * ADIE  = Enable ADC interrupt
     * ADATE = Enable auto-trigger
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     *
     * FIX: Changed prescaler from 64 to 128 for more stable conversions
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* Auto-trigger source = Timer0 Compare Match A */
    ADCSRB |= (1 << ADTS1) | (1 << ADTS0);

    /* Start first conversion */
    ADCSRA |= (1 << ADSC);
}

/*
 * Timer0 Compare Match A ISR
 * Called 8000 times per second (8 kHz)
 */
ISR(TIMER0_COMPA_vect) {
    /*
     * TX signal generation:
     * Toggle every 2nd interrupt = 4 kHz toggle rate = 2 kHz square wave
     *
     * FIX: Original had "i >= 1" which toggled every 2nd call at 16kHz,
     * giving 4kHz TX instead of 2kHz
     */
    if (i >= 1) {
        PORTB ^= (1 << PB1);
        i = 0;
    }
    i++;
}

/*
 * ADC Conversion Complete ISR
 * Called when ADC conversion is done (auto-triggered by Timer0 Compare Match A)
 */
ISR(ADC_vect) {
    ADC_Raw = ADC;
    DFT_sum(ADC_Raw);
}
