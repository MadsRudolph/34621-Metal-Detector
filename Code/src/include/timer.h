/*
 * timer.h - Timer konfiguration for Metal Detektor
 *
 * Timer0: Genererer 8kHz interrupt til TX signal og ADC trigger
 * Timer1: Sleep mode styring
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/* ============ GLOBALE VARIABLE ============ */
// Flag der indikerer TX rising edge (bruges til DFT synkronisering)
extern volatile uint8_t rising_edge_Flag;

// Sleep mode flag
extern volatile uint8_t do_sleep;

/* ============ FUNKTIONER ============ */

/*
 * Initialiserer Timer0 i CTC mode
 * Genererer 8kHz interrupt som:
 *   1. Toggler TX pin hver 2. gang = 2kHz firkantbølge
 *   2. Trigger ADC auto-sampling
 */
void timer0_init(void);

/*
 * Initialiserer Timer1 til sleep mode styring
 * Phase correct PWM mode med periodisk interrupt
 */
void Timer1_init(void);

#endif /* TIMER_H */
