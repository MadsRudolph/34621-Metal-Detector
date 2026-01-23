/*
 * adc.h - ADC konfiguration for Metal Detektor
 *
 * ADC auto-triggers fra Timer0 Compare Match A = 8kHz sampling
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* --- Globale variable --- */
// Rå ADC værdi (10-bit, 0-1023)
extern int16_t ADC_Raw;

/* --- Funktioner --- */

/*
 * Initialiserer ADC med:
 *   - AVCC som reference (5V)
 *   - Auto-trigger fra Timer0 Compare Match A
 *   - Interrupt når konvertering er færdig
 */
void adc_init(void);

#endif /* ADC_H */
