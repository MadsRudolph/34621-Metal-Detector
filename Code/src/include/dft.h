/*
 * dft.h - DFT beregning for Metal Detektor
 *
 * Implementerer en optimeret Goertzel-lignende DFT ved 2kHz
 * Udnytter at samples er periodiske med Pi/2 faseskift
 */

#ifndef DFT_H
#define DFT_H

#include <stdint.h>

/* --- Globale variable --- */
// DFT resultater (buffer værdier efter et komplet vindue)
extern volatile int32_t Re_buff;    // Real del
extern volatile int32_t Im_buff;    // Imaginær del

// Beregnede værdier
extern volatile uint16_t mag;       // Magnitude
extern volatile int16_t ang;        // Fase i grader (-180 til +180)

// Flag der indikerer at en DFT er færdig
extern volatile uint8_t DFT_done;

/* --- Funktioner --- */

/*
 * Akkumulerer en ADC sample til DFT
 * Kaldes fra ADC ISR for hver sample (8kHz)
 * Når N samples er akkumuleret, sættes DFT_done flag
 */
void DFT_sum(int16_t ADC_Raw);

/*
 * Beregner magnitude og fase fra Re_buff og Im_buff
 * Kaldes fra main loop når DFT_done er sat
 */
void DFT_Calc(void);

#endif /* DFT_H */
