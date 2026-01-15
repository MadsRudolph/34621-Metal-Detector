/**
 * @file encoder.c
 * @brief Turn counting via ITR8307 optical sensor
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of interrupt-driven turn counting with software debounce.
 *          Uses INT0 falling edge interrupt to detect white stripe passing sensor.
 */

#include "encoder.h"
#include "config.h"
#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/**
 * @brief Turn counter (volatile for ISR access)
 */
static volatile uint16_t g_turn_count = 0;

/**
 * @brief New turn flag set by ISR, cleared by main loop
 */
static volatile bool g_new_turn = false;

/**
 * @brief Timestamp of last valid trigger for debounce
 */
static volatile uint32_t g_last_trigger = 0;

/**
 * @brief Minimum time between valid triggers in milliseconds
 *
 * @details At 60 RPM (1 turn/sec), period = 1000ms
 *          At 300 RPM (5 turns/sec), period = 200ms
 *          20ms safely filters noise while allowing fast winding.
 */
#define ENCODER_DEBOUNCE_MS  20

/**
 * @brief INT0 external interrupt handler
 *
 * @details Called on falling edge when white stripe enters sensor field.
 *          Increments turn count and sets new-turn flag.
 *          Debounce filtering is done in encoder_check_turn() in main loop.
 */
ISR(INT0_vect)
{
    g_turn_count++;
    g_new_turn = true;
}

void encoder_init(void)
{
    /* Configure PD2 as input with internal pull-up */
    ENCODER_DDR &= ~(1 << ENCODER_PIN);
    ENCODER_PORT |= (1 << ENCODER_PIN);

    /* INT0 falling edge trigger: ISC01=1, ISC00=0 */
    EICRA = (1 << ISC01);

    /* Enable INT0 interrupt */
    EIMSK = (1 << INT0);
}

bool encoder_check_turn(void)
{
    /* No new turn pending */
    if (!g_new_turn) {
        return false;
    }

    uint32_t now = millis();

    /* Check if trigger is within debounce window (likely noise) */
    if (now - g_last_trigger < ENCODER_DEBOUNCE_MS) {
        /* Too fast - filter as noise */
        g_new_turn = false;

        /* Decrement the count that was erroneously incremented by noise */
        cli();
        if (g_turn_count > 0) {
            g_turn_count--;
        }
        sei();

        return false;
    }

    /* Valid turn - update timestamp and clear flag */
    g_last_trigger = now;
    g_new_turn = false;

    return true;
}

uint16_t encoder_get_count(void)
{
    uint16_t count;

    /* Atomic read of 16-bit counter */
    cli();
    count = g_turn_count;
    sei();

    return count;
}

void encoder_reset(void)
{
    /* Atomically reset counter and flag */
    cli();
    g_turn_count = 0;
    g_new_turn = false;
    sei();
}

bool encoder_new_turn(void)
{
    return g_new_turn;
}

void encoder_clear_flag(void)
{
    g_new_turn = false;
}
