/*
 * button.c - Knap konfiguration for Metal Detektor
 *
 * Hardware:
 *   Pin D2 = Start/Stop (Krav 9a)
 *   Pin D3 = Kalibrering (Krav 9b)
 *   Pin D4 = Skift mellem DFT og Debug skærm
 */

#include <avr/io.h>
#include "button.h"
#include "config.h"

/* ============ BUTTON INIT ============ */
void button_init(void) {
    // Pin D4 som input (debug skærm knap)
    DDRD &= ~(1 << BTN_DEBUG);
    // Aktiver intern pull-up modstand (knap forbinder til GND)
    PORTD |= (1 << BTN_DEBUG);

    // Pin D2 som input - Start/Stop knap (Krav 9a)
    DDRD &= ~(1 << BTN_START_STOP);
    PORTD |= (1 << BTN_START_STOP);

    // Pin D3 som input - Kalibrering knap (Krav 9b)
    DDRD &= ~(1 << BTN_CALIBRATE);
    PORTD |= (1 << BTN_CALIBRATE);
}

/* ============ BUTTON READ FUNKTIONER ============ */
// Returnerer 1 hvis IKKE trykket, 0 hvis trykket (active low)

uint8_t button_read_start_stop(void) {
    return (PIND >> BTN_START_STOP) & 1;
}

uint8_t button_read_calibrate(void) {
    return (PIND >> BTN_CALIBRATE) & 1;
}

uint8_t button_read_debug(void) {
    return (PIND >> BTN_DEBUG) & 1;
}
