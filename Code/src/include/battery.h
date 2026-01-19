/*
 * battery.h - Batteri spændingsmonitor for Metal Detektor
 *
 * Læser batteri spænding via spændingsdeler på ADC1 (A1)
 * Hardware: 46.3k/10k divider giver 0.1776x af batterspænding
 */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

/* ============ GLOBALE VARIABLE ============ */
extern uint16_t battery_voltage;    // Filtreret spænding i mV (f.eks. 8500 = 8.5V)

/* ============ FUNKTIONER ============ */

/*
 * Initialiser batteri monitor
 * (Ingen speciel init nødvendig - ADC allerede konfigureret)
 */
void battery_init(void);

/*
 * Læs batteri spænding
 * Skifter til ADC1, læser, skifter tilbage til ADC0
 * Returns: Spænding i mV (f.eks. 8500 = 8.5V)
 */
uint16_t battery_read(void);

/*
 * Beregn batteri procent
 * 6V = 0%, 9V = 100%
 * Returns: Procent 0-100
 */
uint8_t battery_percent(void);

#endif /* BATTERY_H */
