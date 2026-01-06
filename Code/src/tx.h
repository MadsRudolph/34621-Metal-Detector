/**
 * tx.h
 * TX Signal Generator using Timer1 (2kHz square wave on OC1A)
 *
 * Hardware: OC1A = PB5 = Arduino Pin 11
 * Output: 2kHz 50% duty cycle square wave
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#ifndef TX_H
#define TX_H

/**
 * Initialize Timer1 for 2kHz square wave output on OC1A (Pin 11)
 * Uses CTC mode with toggle on compare match
 * Does NOT start the output - call tx_start() after init
 */
void tx_init(void);

/**
 * Start TX signal generation (enable Timer1 output)
 */
void tx_start(void);

/**
 * Stop TX signal generation (disable Timer1 output)
 * Sets output pin LOW
 */
void tx_stop(void);

#endif /* TX_H */
