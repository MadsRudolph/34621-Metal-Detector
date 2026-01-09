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

/* DFT buffers - for debug access */
extern volatile int32_t Re_buff;
extern volatile int32_t Im_buff;

/* Flag: set to 1 when new DFT is ready */
extern volatile uint8_t DFT_done;

/* Raw magnitude and phase */
extern volatile uint16_t magnitude;
extern volatile int16_t phase_deg;

/* IIR-filtered values - use these for display and classification */
extern volatile uint16_t magnitude_filt;
extern volatile int16_t phase_filt;

/* Function prototypes */
void DFT_sum(int16_t ADC_Raw);
void DFT_Calc(void);

#endif
