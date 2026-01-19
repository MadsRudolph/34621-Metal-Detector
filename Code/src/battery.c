/*
 * battery.c - Batteri spændingsmonitor for Metal Detektor
 *
 * Læser batteri spænding via spændingsdeler på ADC1 (A1)
 *
 * Hardware:
 *   VIN (9V) ──┬──> To Arduino regulator
 *              │
 *         R1  ─┴─  46.3kΩ
 *              │
 *              ├──────> Pin A1 (ADC1)
 *              │
 *         R2  ─┴─  10kΩ
 *              │
 *             GND
 *
 * Beregning:
 *   Divider ratio: 10/(46.3+10) = 0.1776
 *   Ved 9V: 9 × 0.1776 = 1.60V (sikkert for ADC)
 *   Ved 6V: 6 × 0.1776 = 1.07V
 *   ADC formel: battery_mV = (ADC_reading * 5000 / 1024) * 563 / 100
 */

#include <avr/io.h>
#include <util/delay.h>
#include "include/battery.h"
#include "include/config.h"

/* ============ GLOBALE VARIABLE ============ */
uint16_t battery_voltage = 0;       // Filtreret spænding i mV

/* ============ LOKALE VARIABLE ============ */
static uint8_t filter_initialized = 0;

/* ============ FUNKTIONER ============ */

void battery_init(void) {
    // Ingen speciel init nødvendig - ADC allerede konfigureret i adc.c
    // Vi bruger manuel konvertering på ADC1 kanal
    filter_initialized = 0;
    battery_voltage = 0;
}

uint16_t battery_read(void) {
    // Gem ADCSRA og slå auto-trigger og interrupt fra for at undgå race
    uint8_t old_adcsra = ADCSRA;
    ADCSRA &= ~((1 << ADATE) | (1 << ADIE));

    // Vent på evt. igangværende konvertering
    while (ADCSRA & (1 << ADSC));

    // Gem nuværende ADMUX indstillinger
    uint8_t old_admux = ADMUX;

    // Skift til ADC1, behold AVCC reference
    ADMUX = (1 << REFS0) | (1 << MUX0);

    // Vent på settling (ADC sample/hold)
    _delay_us(100);

    // Start manuel konvertering
    ADCSRA |= (1 << ADSC);

    // Vent på konvertering færdig
    while (ADCSRA & (1 << ADSC));

    // Læs resultat
    uint16_t raw = ADC;

    // Gendan original ADMUX og ADCSRA (inkl. ADATE og ADIE)
    ADMUX = old_admux;
    ADCSRA = old_adcsra;

    // Beregn spænding i mV
    // Formel: raw * 5000 / 1024 * 563 / 100 (for 46.3k/10k divider)
    // Divider ratio = 10/(46.3+10) = 0.1776, inverse = 5.63
    uint32_t adc_mv = ((uint32_t)raw * 5000) / 1024;
    uint16_t voltage = (adc_mv * 563) / 100;

    // IIR filter for glatning (alpha = 0.125 = 1/8)
    if (!filter_initialized) {
        battery_voltage = voltage;
        filter_initialized = 1;
    } else {
        // battery_voltage = battery_voltage * 7/8 + voltage * 1/8
        battery_voltage = battery_voltage - (battery_voltage >> 3) + (voltage >> 3);
    }

    return battery_voltage;
}

uint8_t battery_percent(void) {
    // Clamp til gyldigt område
    if (battery_voltage <= BATTERY_LOW_MV) {
        return 0;
    }
    if (battery_voltage >= BATTERY_FULL_MV) {
        return 100;
    }

    // Lineær interpolation: (voltage - 6000) / (9000 - 6000) * 100
    uint16_t range = BATTERY_FULL_MV - BATTERY_LOW_MV;
    uint16_t offset = battery_voltage - BATTERY_LOW_MV;
    return (uint8_t)(((uint32_t)offset * 100) / range);
}
