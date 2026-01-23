/*
 * button.h - Knap konfiguration for Metal Detektor
 *
 * Hardware:
 *   Pin D2 = Start/Stop (Krav 9a)
 *   Pin D3 = Kalibrering (Krav 9b)
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

/* --- Funktioner --- */

/*
 * Initialiserer knap pins som input med pull-up
 * Knapper forbinder til GND når trykket (active low)
 */
void button_init(void);

/*
 * Læs knap tilstand (returnerer 1 hvis IKKE trykket, 0 hvis trykket)
 */
uint8_t button_read_start_stop(void);
uint8_t button_read_calibrate(void);

#endif /* BUTTON_H */
