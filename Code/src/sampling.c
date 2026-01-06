/**
 * sampling.c
 * Sampling implementation with Timer3 ISR and double buffering
 */

#include "sampling.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Double buffer for samples */
static volatile uint16_t buffer_a[SAMPLE_BUFFER_SIZE];
static volatile uint16_t buffer_b[SAMPLE_BUFFER_SIZE];
static volatile uint16_t *active_buffer = buffer_a;
static volatile uint16_t *ready_buffer = buffer_b;

/* Buffer management */
static volatile uint8_t sample_index = 0;
static volatile uint8_t buffer_ready_flag = 0;

void adc_init(void)
{
    ADMUX = (1 << REFS0) | (ADC_CHANNEL & 0x07);
    ADCSRB = 0;
    ADCSRA = (1 << ADEN) | ADC_PRESCALER_BITS;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    DIDR0 |= (1 << ADC_CHANNEL);
}

void sampling_timer_init(void)
{
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    OCR3A = SAMPLE_OCR3A_VALUE;
    TIFR3 = (1 << OCF3A);
}

void sampling_start(void)
{
    sample_index = 0;
    buffer_ready_flag = 0;
    TCCR3B = (1 << WGM32) | (1 << CS30);
    TIMSK3 = (1 << OCIE3A);
}

void sampling_stop(void)
{
    TCCR3B = 0;
    TIMSK3 = 0;
}

uint8_t sampling_buffer_ready(void)
{
    uint8_t ready;
    cli();
    ready = buffer_ready_flag;
    buffer_ready_flag = 0;
    sei();
    return ready;
}

volatile uint16_t* sampling_get_buffer(void)
{
    return ready_buffer;
}

ISR(TIMER3_COMPA_vect)
{
    /* Start ADC conversion */
    ADCSRA |= (1 << ADSC);

    /* Wait for conversion */
    while (ADCSRA & (1 << ADSC));

    /* 
     * Read ADC result - MUST read ADCL first, then ADCH
     * Using ADC macro directly can fail in ISR context!
     */
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    uint16_t adc_value = low | (high << 8);

    /* Store sample */
    active_buffer[sample_index] = adc_value;
    sample_index++;

    if (sample_index >= SAMPLE_BUFFER_SIZE) {
        sample_index = 0;
        volatile uint16_t *temp = active_buffer;
        active_buffer = ready_buffer;
        ready_buffer = temp;
        buffer_ready_flag = 1;
    }
}
