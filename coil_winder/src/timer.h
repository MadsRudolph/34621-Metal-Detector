/**
 * @file timer.h
 * @brief Millisecond timer using Timer0
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Provides millisecond timing services using Timer0 in CTC mode.
 *          Timer0 is configured for 1kHz interrupt rate (1ms resolution).
 *          Used for button debouncing, display refresh timing, and encoder debounce.
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/**
 * @brief Initialize Timer0 for millisecond counting
 *
 * @details Configures Timer0 in CTC mode with prescaler 64.
 *          At 16MHz: 16MHz / 64 / 250 = 1000Hz (1ms period).
 *          Enables TIMER0_COMPA interrupt for incrementing counter.
 *
 * @note Must be called before sei() to ensure proper interrupt setup.
 */
void timer_init(void);

/**
 * @brief Get current millisecond count
 *
 * @details Returns the number of milliseconds elapsed since timer_init().
 *          Uses atomic access (cli/sei) to safely read 32-bit counter.
 *          Counter will overflow after ~49.7 days of continuous operation.
 *
 * @return uint32_t Milliseconds since timer initialization
 */
uint32_t millis(void);

/**
 * @brief Blocking delay for specified milliseconds
 *
 * @details Busy-waits until the specified time has elapsed.
 *          Uses millis() internally, so Timer0 must be running.
 *
 * @param ms Number of milliseconds to delay (0-65535)
 *
 * @warning This is a blocking function. Use sparingly in main loop.
 */
void delay_ms(uint16_t ms);

#endif /* TIMER_H */
