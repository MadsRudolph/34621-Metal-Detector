/*
 * tx.h
 * TX signal generation and ADC sampling control for Arduino Nano (ATmega328P)
 *
 * Funktioner til at generere 2 kHz TX signal på Pin 9 og sample ADC ved 8 kHz på A0
 *
 * ÆNDRING: Fjernet ATmega2560 (Mega) support - kun Nano.
 *
 * ATmega328P Datasheet Reference (Rev. 7810D–AVR–01/15):
 * ======================================================
 * Timer/Counter0:  Section 14, Page 74-88
 * ADC:             Section 23, Page 205-220
 * Se tx_rx.c for detaljerede register referencer.
 */

#ifndef TX_H
#define TX_H

#include <avr/io.h>

/*
 * ============================================================================
 * FEJL I ORIGINAL tx.h:
 * ============================================================================
 *
 * Headerfilen var TOM - ingen funktionsprototyper.
 *
 * HVORFOR DET ER FORKERT:
 * - main.c kalder timer1_init(), adc_init(), sampling_start()
 * - Uden prototyper får compileren ingen type-checking
 * - Kan føre til subtile bugs hvis argumenter ikke matcher
 *
 * KORREKT: Deklarer alle funktioner der bruges fra andre filer
 * ============================================================================
 */

/* ===== Timer Initialisering ===== */

/*
 * timer0_init() - Initialiser TX signal generation
 *
 * Sætter Timer0 op til 8 kHz interrupt og genererer 2 kHz TX signal på Pin 9 (PB1).
 *
 * Konfiguration:
 * - CTC mode (Clear Timer on Compare Match)   - Datasheet: Section 14.7.2, Page 79
 * - Prescaler = 8                              - Datasheet: Table 14-9, Page 87
 * - OCR0A = 249 → 16MHz / (8 * 250) = 8 kHz   - Datasheet: Section 14.9.4, Page 87
 * - TX toggle hver 2. interrupt → 2 kHz square wave
 *
 * FJERNET: timer1_init() wrapper - navn var forvirrende da det bruger Timer0.
 */
void timer0_init(void);

/* ===== ADC Kontrol ===== */

/*
 * adc_init() - Initialiser ADC
 *
 * Konfigurerer ADC med:
 * - AVCC reference (5V)                        - Datasheet: Table 23-3, Page 217
 * - Kanal 0 (A0)                               - Datasheet: Table 23-4, Page 218
 * - Prescaler 128 (125 kHz ADC clock)          - Datasheet: Table 23-5, Page 219
 * - Interrupt enabled                          - Datasheet: Section 23.9.2, Page 218
 */
void adc_init(void);

/*
 * sampling_start() - Start ADC sampling
 *
 * Aktiverer 8 kHz ADC sampling synkroniseret med TX signal.
 * Skal kaldes efter timer0_init() og adc_init().
 */
void sampling_start(void);

/*
 * sampling_stop() - Stop ADC sampling
 *
 * Stopper ADC sampling. TX signal fortsætter.
 */
void sampling_stop(void);

#endif /* TX_H */
