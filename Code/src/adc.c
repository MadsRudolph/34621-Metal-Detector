/*
 * adc.c - ADC konfiguration for Metal Detektor
 *
 * ADC auto-triggers fra Timer0 Compare Match A = 8kHz sampling
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "adc.h"
#include "timer.h"
#include "dft.h"

/* ============ GLOBALE VARIABLE ============ */
int16_t ADC_Raw = 0;    // Rå ADC værdi (signed for DFT beregning)

/* ============ ADC INIT ============ */
void adc_init(void) {
    // Reference = AVCC (intern 5V reference)
    ADMUX = (1 << REFS0);

    /*
     * ADC Kontrol:
     *   ADEN  = Aktiver ADC
     *   ADIE  = Aktiver ADC interrupt
     *   ADATE = Aktiver auto-trigger
     *   ADPS  = Prescaler 64 (250kHz ADC clock ved 16MHz)
     */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1);

    // Auto-trigger kilde = Timer0 Compare Match A
    ADCSRB |= (1 << ADTS1) | (1 << ADTS0);

    // Start første konvertering
    ADCSRA |= (1 << ADSC);
}

/* ============ ADC INTERRUPT ============ */
/*
 * ADC Conversion Complete Interrupt
 *
 * Denne ISR kører automatisk når ADC er færdig med en konvertering.
 * ADC'en er sat op til auto-trigger fra Timer0 Compare Match A,
 * så den sampler med samme 8kHz rate som Timer0.
 *
 * Formål:
 *   1. Læse ADC værdien (0-1023, 10-bit)
 *   2. Føde samples til DFT når rising_edge_Flag er sat
 *
 * Vi sampler kun til DFT når rising_edge_Flag == 1, hvilket sikrer
 * at DFT'en starter synkroniseret med TX rising edge.
 */
ISR(ADC_vect) {
    ADC_Raw = ADC;              // Gem ADC værdi (10-bit, 0-1023)

    if (rising_edge_Flag == 1) {
        DFT_sum(ADC_Raw);       // Send sample til DFT akkumulator
    }
}
