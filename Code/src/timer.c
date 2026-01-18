/*
 * timer.c - Timer konfiguration for Metal Detektor
 *
 * Timer0: Genererer 8kHz interrupt til TX signal og ADC trigger
 * Timer1: Sleep mode styring
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "config.h"

/* ============ GLOBALE VARIABLE ============ */
volatile uint8_t rising_edge_Flag = 0;  // Flag der indikerer TX rising edge
volatile uint8_t do_sleep = 0;          // Sleep mode flag

// Lokal tæller til TX toggle
static uint8_t i = 0;

/* ============ TIMER0 INIT ============ */
void timer0_init(void) {
    // Sæt TX pin som output
    DDRB |= (1 << TX_PIN);

    // CTC mode (Clear Timer on Compare Match) - side 86 i datasheet
    TCCR0A |= (1 << WGM01);

    // Prescaler 8: CS01=1, CS00=0
    TCCR0B |= (1 << CS01);

    // Compare match værdi: 16MHz / 8 / 250 = 8kHz interrupt
    OCR0A = 249;

    // Enable Timer0 Compare Match A interrupt
    TIMSK0 = (1 << OCIE0A);
}

/* ============ TIMER1 INIT (SLEEP MODE) ============ */
void Timer1_init(void) {
    // Phase correct PWM mode, 10-bit
    TCCR1A |= (1 << WGM10) | (1 << WGM11);

    // Prescaler 64 (timer clock = 16MHz/64 = 250kHz)
    TCCR1B = (1 << CS11) | (1 << CS10);

    // Set compare value
    OCR1B = 156;

    // Enable Compare Match B Interrupt
    TIMSK1 |= (1 << OCIE1B);
}

/* ============ TIMER0 INTERRUPT ============ */
/*
 * Timer0 Compare Match A Interrupt
 *
 * Denne ISR kører med 8kHz (16MHz / 8 prescaler / 250 = 8000 Hz)
 *
 * Formål:
 *   1. Generere 2kHz TX signal til sendespolen
 *   2. Sætte flag når TX går høj (rising edge) for at synkronisere DFT
 *
 * Timing:
 *   - ISR kører hver 125µs (8kHz)
 *   - Vi toggler TX pin hver 2. gang = 250µs mellem toggles
 *   - 250µs HIGH + 250µs LOW = 500µs periode = 2kHz firkantbølge
 *
 * rising_edge_Flag bruges til at starte DFT sampling præcis når TX går høj,
 * så DFT'en er synkroniseret med TX signalet (fase 0° ved TX rising edge)
 */
ISR(TIMER0_COMPA_vect) {
    i++;                            // Tæl antal interrupts
    if (i >= 2) {                   // Hver 2. interrupt (4kHz toggle rate)
        PORTB ^= (1 << TX_PIN);     // Toggle TX pin (XOR flipper bit)
        i = 0;                      // Nulstil tæller

        // Tjek om TX lige gik høj (rising edge)
        if (PORTB & (1 << TX_PIN)) {
            rising_edge_Flag = 1;   // Signal til ADC ISR: start ny DFT periode
        }
    }
}

/* ============ TIMER1 INTERRUPT (SLEEP MODE) ============ */
ISR(TIMER1_COMPB_vect) {
    if (!do_sleep) {
        do_sleep = 1;
    } else {
        do_sleep = 0;
    }
}
