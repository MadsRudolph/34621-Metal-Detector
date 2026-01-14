/**
 * @file timer.c
 * @brief Millisecond timer implementation using Timer0
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 */

#include "timer.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * @brief Global millisecond counter
 * @details Volatile for ISR access, incremented every 1ms by Timer0 interrupt.
 */
static volatile uint32_t g_millis = 0;

/**
 * @brief Timer0 Compare Match A interrupt handler
 * @details Called every 1ms to increment the millisecond counter.
 */
ISR(TIMER0_COMPA_vect)
{
    g_millis++;
}

void timer_init(void)
{
    /* CTC mode (Clear Timer on Compare Match) */
    TCCR0A = (1 << WGM01);

    /* Prescaler 64: CS01 + CS00 = 011 */
    TCCR0B = (1 << CS01) | (1 << CS00);

    /* Compare value for 1kHz: 16MHz / 64 / 250 = 1000Hz */
    OCR0A = 249;

    /* Enable Timer0 Compare Match A interrupt */
    TIMSK0 = (1 << OCIE0A);
}

uint32_t millis(void)
{
    uint32_t ms;

    /* Atomic read of 32-bit value */
    cli();
    ms = g_millis;
    sei();

    return ms;
}

void delay_ms(uint16_t ms)
{
    uint32_t start = millis();
    while (millis() - start < ms);
}
