/*
 * timer.c - Timer konfiguration for Metal Detektor
 *
 * Timer0: Genererer 8kHz interrupt til TX signal og ADC trigger
 * Timer1: Sleep mode styring
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "include/timer.h"
#include "include/config.h"
#include "include/detection.h"

/* --- Globale variable --- */
volatile uint8_t rising_edge_Flag = 0;  // Flag der indikerer TX rising edge
volatile uint8_t do_sleep = 0;          // Sleep mode flag

// Lokal tæller til TX toggle
static uint8_t i = 0;

/* --- Timer0 Init --- */
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

/* --- Timer1 Init (Sleep Mode) --- */
void Timer1_init(void) {
    // Phase correct PWM mode, 10-bit
    TCCR1A |= (1 << WGM10) | (1 << WGM11);

    // Prescaler 64 (timer clock = 16MHz/64 = 250kHz)
    TCCR1B = (1 << CS11) | (1 << CS10);

    // Set compare value
    OCR1B = 512; //sleep every 2 seconds (250kHz/512 = ~488Hz; 488/256 = ~1.9Hz)

    // Enable Compare Match B Interrupt
    TIMSK1 |= (1 << OCIE1B);

    // Konfigurer SLEEP_PIN til sleep mode styring
    DDRD |= (1 << SLEEP_PIN); // Sæt SLEEP_PIN som output

}

/* --- Timer0 Interrupt --- */
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

/* --- Timer1 Interrupt (Sleep Mode) --- */
ISR(TIMER1_COMPB_vect) {
    // Kun toggle sleep mode når detektor er aktiv
    // Når stoppet holder detection.c SLEEP_PIN høj permanent
    if (!detection_active) {
        return;
    }

    if (!do_sleep) {
        do_sleep = 1;
        PORTD |= (1 << SLEEP_PIN);   // H-bridge sleep
    } else {
        do_sleep = 0;
        PORTD &= ~(1 << SLEEP_PIN);  // H-bridge aktiv
    }
}
