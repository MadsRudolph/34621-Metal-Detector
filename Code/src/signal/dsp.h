/*
 * dsp.h
 * Single-bin DFT ved 2kHz
 *
 * Optimeret til 4x oversampling - koefficienter er {1, 0, -1, 0}
 * Beregner magnitude og fase fra RX signal
 */

#ifndef DSP_H
#define DSP_H

#include <stdint.h>

/* Rå DFT akkumulator resultater (efter 64 samples) */
extern volatile int32_t Re_buff;
extern volatile int32_t Im_buff;
extern volatile uint8_t DFT_done;

/* Beregnede resultater fra DFT_Calc() - rå værdier */
extern volatile uint16_t magnitude;
extern volatile int16_t phase_deg;

/* IIR filtrerede resultater - brug disse til display/detektor */
extern volatile uint16_t magnitude_filt;
extern volatile int16_t phase_filt;

/* Kald fra ADC ISR for hver sample */
void DFT_sum(uint16_t ADC_Raw);

/* Kald efter DFT_done==1 for at beregne magnitude og fase */
void DFT_Calc(void);

#endif
