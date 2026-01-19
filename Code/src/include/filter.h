/*
 * filter.h - IIR filter for Metal Detektor
 *
 * Lavpasfilter der glatter DFT resultater for stabil displayvisning
 */

#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

/* ============ GLOBALE VARIABLE ============ */
// Filtrerede værdier til display og klassificering
extern volatile uint32_t mag_filtered;  // Filtreret magnitude
extern volatile int16_t ang_filtered;   // Filtreret fase i grader

/* ============ FUNKTIONER ============ */

/*
 * IIR lavpasfilter til magnitude og fase
 *
 * Implementerer: y[n] = 0.7*y[n-1] + 0.3*x[n]
 * Undgår floats ved at bruge heltalsdivision: (7*y + 3*x) / 10
 *
 * Første gang funktionen kaldes, initialiseres filteret med
 * den aktuelle måling for at undgå startup-forsinkelse.
 */
void IIR_Filt(void);

#endif /* FILTER_H */
