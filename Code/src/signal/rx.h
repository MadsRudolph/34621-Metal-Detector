/*
 * rx.h
 * ADC sampling for RX signal
 *
 * Uses Timer1 Compare Match B to trigger ADC (phase-locked to TX)
 * 4 samples per TX cycle = 8kHz sampling
 * Fills 64-sample buffer, sets flag when ready
 */

#ifndef RX_H
#define RX_H

#include <stdint.h>

#define ADC_BUFFER_SIZE 64

/* Sample buffer - filled by ISR */
extern volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

/* Flag: 1 = buffer ready for processing, clear after use */
extern volatile uint8_t buffer_ready;

/* Initialize ADC with Timer1 Compare B auto-trigger */
void adc_init(void);

/* Start sampling (enable ADC interrupt) */
void sampling_start(void);

/* Stop sampling (disable ADC interrupt) */
void sampling_stop(void);

#endif
