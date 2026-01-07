/*
 * rx.c
 * ADC sampling for RX signal
 *
 * Timer1 Compare Match B triggers ADC (phase-locked to TX)
 * Timer1 counts 0-3999 at 16MHz = 4kHz cycle rate
 * We need 8kHz sampling = 2 samples per timer cycle
 * OCR1B alternates between 999 and 2999 for even spacing
 */

#include "rx.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Sample buffer */
volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Buffer ready flag */
volatile uint8_t buffer_ready = 0;

/* Current position in buffer (0-63) */
static volatile uint8_t sample_index = 0;

/* Sample points within Timer1 cycle (0-3999) */
/* Two samples per cycle: at 1/4 and 3/4 points */
#define SAMPLE_POINT_A  999
#define SAMPLE_POINT_B  2999

void adc_init(void)
{
    /* Reference = AVCC (5V), channel 0 */
    ADMUX = (1 << REFS0);

    /*
     * ADC Control:
     * ADEN  = Enable ADC
     * ADIE  = Enable ADC interrupt
     * ADATE = Enable auto-trigger
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* Auto-trigger source = Timer1 Compare Match B */
    ADCSRB = (1 << ADTS2) | (1 << ADTS0);

    /* Set initial compare point */
    OCR1B = SAMPLE_POINT_A;
}

void sampling_start(void)
{
    sample_index = 0;
    buffer_ready = 0;
    OCR1B = SAMPLE_POINT_A;

    /* Enable global interrupts */
    sei();

    /* Start first conversion */
    ADCSRA |= (1 << ADSC);
}

void sampling_stop(void)
{
    /* Disable ADC interrupt */
    ADCSRA &= ~(1 << ADIE);
}

/*
 * ADC Conversion Complete ISR
 * Stores sample, updates OCR1B for next trigger point
 */
ISR(ADC_vect)
{
    if (!buffer_ready) {
        /* Store sample */
        adc_buffer[sample_index] = ADC;
        sample_index++;

        if (sample_index >= ADC_BUFFER_SIZE) {
            sample_index = 0;
            buffer_ready = 1;
        }
    }

    /* Alternate trigger point for next sample */
    if (OCR1B == SAMPLE_POINT_A) {
        OCR1B = SAMPLE_POINT_B;
    } else {
        OCR1B = SAMPLE_POINT_A;
    }
}
