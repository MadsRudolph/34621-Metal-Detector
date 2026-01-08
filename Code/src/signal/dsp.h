/*
 * dsp.h
 * Single-bin DFT ved 2kHz for Arduino Nano (ATmega328P)
 *
 * Optimeret til 4x oversampling - koefficienter er {1, 0, -1, 0}
 * Beregner magnitude og fase fra RX signal på A0
 */

#ifndef DSP_H
#define DSP_H

#include <stdint.h>

/*
 * ============================================================================
 * FEJL I ORIGINAL dsp.h:
 * ============================================================================
 *
 * Headerfilen var TOM - ingen extern deklarationer eller funktionsprototyper.
 *
 * HVORFOR DET ER FORKERT:
 * - main.c og andre filer inkluderer "dsp.h" for at få adgang til DSP variabler
 * - Uden extern deklarationer ved compileren ikke at variablerne eksisterer
 * - Resulterer i "undefined reference" linker fejl
 * - Funktionsprototyper manglede også, så compileren kunne ikke tjekke argumenter
 *
 * KORREKT: Deklarer alle variabler som 'extern' og tilføj funktionsprototyper
 * ============================================================================
 */

/* ===== DFT Resultater (eksporteret fra dsp.c) ===== */

/* Rå DFT komponenter - bruges til debug */
extern volatile int32_t Re_buff;
extern volatile int32_t Im_buff;

/* Flag: sættes til 1 når ny DFT er klar */
extern volatile uint8_t DFT_done;

/* Rå magnitude og fase */
extern volatile uint16_t magnitude;
extern volatile int16_t phase_deg;

/* IIR-filtrerede værdier - brug disse til display og klassifikation */
extern volatile uint16_t magnitude_filt;
extern volatile int16_t phase_filt;

/* ===== Funktionsprototyper ===== */

/*
 * DFT_sum() - Kaldes fra ADC ISR for hver sample
 *
 * Akkumulerer DFT over 64 samples ved hjælp af 4x oversampling trick.
 * Sætter DFT_done = 1 når vindue er komplet.
 *
 * @param ADC_Raw: Rå ADC værdi (0-1023)
 */
void DFT_sum(uint16_t ADC_Raw);

/*
 * DFT_Calc() - Beregn magnitude og fase fra akkumuleret DFT
 *
 * Skal kaldes fra main loop når DFT_done == 1.
 * Beregner magnitude, phase_deg og anvender IIR filter.
 */
void DFT_Calc(void);

#endif /* DSP_H */
