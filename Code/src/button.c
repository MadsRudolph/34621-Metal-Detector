/*
 * button.c - Knap konfiguration for Metal Detektor
 *
 * Hardware:
 *   Pin D2 = Start/Stop (Krav 9a)
 *   Pin D3 = Kalibrering (Krav 9b)
 */

#include <avr/io.h>
#include "include/button.h"
#include "include/config.h"

/* --- Button Init --- */
void button_init(void) {
    // Pin D2 som input - Start/Stop knap (Krav 9a)
    DDRD &= ~(1 << BTN_START_STOP);
    PORTD |= (1 << BTN_START_STOP);

    // Pin D3 som input - Kalibrering knap (Krav 9b)
    DDRD &= ~(1 << BTN_CALIBRATE);
    PORTD |= (1 << BTN_CALIBRATE);
}

/* --- Button Read funktioner --- */
// Returnerer 1 hvis IKKE trykket, 0 hvis trykket (active low)

uint8_t button_read_start_stop(void) {
    return (PIND >> BTN_START_STOP) & 1;
}

uint8_t button_read_calibrate(void) {
    return (PIND >> BTN_CALIBRATE) & 1;
}
