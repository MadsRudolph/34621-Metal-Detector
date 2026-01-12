/**
 * @file encoder.h
 * @brief Turn counting via ITR8307 optical sensor
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Provides turn counting using the ITR8307/F43 reflective optical sensor.
 *          Uses INT0 (PD2) for interrupt-based edge detection with software debouncing.
 *          The encoder detects white stripes on a dark flywheel rotating past the sensor.
 *
 * @note ITR8307 is a REFLECTIVE sensor - requires white stripe on dark background.
 *       Hardware: 330 ohm on LED anode, 10k pullup on collector output.
 *       Add 100nF capacitor on sensor power for noise filtering from servo PWM.
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize encoder input with INT0 interrupt
 *
 * @details Configures PD2 (INT0) as input with internal pull-up.
 *          Sets INT0 for falling edge detection (EICRA ISC01).
 *          The falling edge occurs when sensor sees the white stripe.
 *
 * @note Must be called before sei() to ensure proper interrupt setup.
 */
void encoder_init(void);

/**
 * @brief Get current turn count
 *
 * @details Returns the cumulative number of turns detected since last reset.
 *          Uses atomic read (cli/sei) to safely access 16-bit counter.
 *
 * @return uint16_t Number of turns counted (0-65535)
 */
uint16_t encoder_get_count(void);

/**
 * @brief Reset turn count to zero
 *
 * @details Atomically clears the turn counter and new-turn flag.
 *          Should be called when starting a new winding operation.
 */
void encoder_reset(void);

/**
 * @brief Check if a new turn was detected (raw, no debounce)
 *
 * @details Returns the raw new-turn flag set by the ISR.
 *          Does not clear the flag - use encoder_clear_flag() after checking.
 *
 * @return true if new turn detected since last clear
 * @return false if no new turn
 *
 * @warning Prefer encoder_check_turn() which includes debounce filtering.
 */
bool encoder_new_turn(void);

/**
 * @brief Clear new turn flag
 *
 * @details Clears the new-turn flag without checking its value.
 *          Used after processing a turn event from encoder_new_turn().
 */
void encoder_clear_flag(void);

/**
 * @brief Check for new turn with debouncing (preferred method)
 *
 * @details Checks for new turn events with 20ms software debounce.
 *          If a trigger occurs within 20ms of the previous valid trigger,
 *          it is considered noise and the count is decremented.
 *
 *          At 300 RPM (5 turns/sec), minimum time between turns is 200ms,
 *          so 20ms debounce safely filters noise without missing real turns.
 *
 * @return true if valid new turn detected
 * @return false if no turn or trigger was filtered as noise
 */
bool encoder_check_turn(void);

#endif /* ENCODER_H */
