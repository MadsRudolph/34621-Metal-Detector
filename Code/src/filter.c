/*
 * filter.c - IIR filter for Metal Detektor
 *
 * Lavpasfilter der glatter DFT resultater for stabil displayvisning
 */

#include "include/filter.h"
#include "include/dft.h"

/* ============ GLOBALE VARIABLE ============ */
volatile uint32_t mag_filtered = 0;     // Filtreret magnitude (32 bit for multiplikation)
volatile int16_t ang_filtered = 0;      // Filtreret fase i grader

// Lokal variabel til initialiseringskontrol
static uint8_t filt_init = 0;

/* ============ IIR FILTER ============ */
/*
 * IIR_Filt - Lavpasfilter for stabile displayværdier
 *
 * Implementerer et simpelt første-ordens IIR filter:
 *   y[n] = α*y[n-1] + (1-α)*x[n]
 *
 * Med α = 0.7 og (1-α) = 0.3 får vi:
 *   y[n] = (7*y[n-1] + 3*x[n]) / 10
 *
 * Dette undgår floating point beregninger og giver
 * et tidskonstant på ca. 3-4 samples.
 */
void IIR_Filt(void) {
    // Ved første kald: initialiser filteret med aktuel måling
    // Dette undgår langsom opstart hvor filteret skal "indhente" signalet
    if (!filt_init) {
        mag_filtered = mag;
        ang_filtered = ang;
        filt_init = 1;
        return;
    }

    // Filtrer magnitude (brug 32-bit for at undgå overflow)
    mag_filtered = (mag_filtered * 7 + (uint32_t)mag * 3) / 10;

    // Filtrer fase
    ang_filtered = (ang_filtered * 7 + ang * 3) / 10;
}
