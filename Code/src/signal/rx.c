/*
 * rx.c
 * ADC sampling til RX signal
 *
 * Timer1 Compare Match B trigger ADC (faselåst til TX)
 * Timer1 tæller 0-3999 ved 16MHz = 4kHz cyklus rate
 * Vi behøver 8kHz sampling = 2 samples per timer cyklus
 * OCR1B skifter mellem 999 og 2999 for jævn fordeling
 */

#include "rx.h"
#include "dsp.h"
#include "../app/debug.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* Sample buffer (beholdt til debugging) */
volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Buffer klar flag */
volatile uint8_t buffer_ready = 0;

/* Nuværende position i buffer (0-63) */
static volatile uint8_t sample_index = 0;

/* Sample punkter inden for Timer1 cyklus (0-3999) */
/* To samples per cyklus: ved 1/4 og 3/4 punkter */
#define SAMPLE_POINT_A  999
#define SAMPLE_POINT_B  2999

/*
 * adc_init
 * Konfigurerer ADC med Timer1 Compare B som trigger
 */
void adc_init(void)
{
    /* Reference = AVCC (5V), kanal 0 (A0) */
    ADMUX = (1 << REFS0);

    /*
     * ADC Kontrol:
     * ADEN  = Aktiver ADC
     * ADIE  = Aktiver ADC interrupt
     * ADATE = Aktiver auto-trigger
     * ADPS  = Prescaler 128 (125kHz ADC clock)
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    /* Auto-trigger kilde = Timer1 Compare Match B */
    ADCSRB = (1 << ADTS2) | (1 << ADTS0);

    /* Sæt første compare punkt */
    OCR1B = SAMPLE_POINT_A;
}

/*
 * sampling_start
 * Starter ADC sampling loop
 */
void sampling_start(void)
{
    sample_index = 0;
    buffer_ready = 0;
    OCR1B = SAMPLE_POINT_A;

    /* Aktiver globale interrupts */
    sei();

    /* Start første konvertering */
    ADCSRA |= (1 << ADSC);
}

/*
 * sampling_stop
 * Stopper ADC sampling
 */
void sampling_stop(void)
{
    /* Deaktiver ADC interrupt */
    ADCSRA &= ~(1 << ADIE);
}

/*
 * ADC Conversion Complete ISR
 * Sender sample til DFT, opdaterer OCR1B til næste trigger
 */
ISR(ADC_vect)
{
    /* Toggle debug pin for oscilloskop timing verification */
    debug_pin_toggle();

    uint16_t sample = ADC;

    /* Log til debug system */
    debug_log_adc_sample(sample);

    /* Ryd Timer1 Compare B flag (skriv 1 for at rydde) */
    TIFR1 = (1 << OCF1B);

    /* Gem i buffer til debugging */
    adc_buffer[sample_index] = sample;
    sample_index++;
    if (sample_index >= ADC_BUFFER_SIZE) {
        sample_index = 0;
        buffer_ready = 1;
    }

    /* Send til DFT akkumulator */
    DFT_sum(sample);

    /* Skift trigger punkt til næste sample */
    if (OCR1B == SAMPLE_POINT_A) {
        OCR1B = SAMPLE_POINT_B;
    } else {
        OCR1B = SAMPLE_POINT_A;
    }
}
