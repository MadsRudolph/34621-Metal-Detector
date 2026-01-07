/*
 * rx.h
 * ADC sampling til RX signal
 *
 * Bruger Timer1 Compare Match B til at trigge ADC (faselåst til TX)
 * 4 samples per TX cyklus = 8kHz sampling
 * Fylder 64-sample buffer, sætter flag når klar
 */

#ifndef RX_H
#define RX_H

#include <stdint.h>

/* Buffer størrelse: 64 samples = 16 TX cykler */
#define ADC_BUFFER_SIZE 64

/* Sample buffer - fyldes af ISR */
extern volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Flag: 1 = buffer klar til behandling, nulstil efter brug */
extern volatile uint8_t buffer_ready;

/* Initialiser ADC med Timer1 Compare B auto-trigger */
void adc_init(void);

/* Start sampling (aktiver ADC interrupt) */
void sampling_start(void);

/* Stop sampling (deaktiver ADC interrupt) */
void sampling_stop(void);

#endif
