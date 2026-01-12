/**
 * @file buzzer.h
 * @brief Buzzer output control for audio feedback
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Provides audio feedback using a piezo buzzer on PB2 (D10).
 *          Generates square wave tones by bit-banging the output pin.
 *          Used for startup beep and completion melody.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

/**
 * @brief Initialize buzzer output pin
 *
 * @details Configures PB2 as output and sets it LOW (buzzer off).
 */
void buzzer_init(void);

/**
 * @brief Generate a beep at specified frequency
 *
 * @details Produces a square wave tone by toggling the buzzer pin.
 *          Uses busy-wait delay for timing (blocking function).
 *
 *          Half-period calculation: 500000us / freq_hz
 *          Number of cycles: freq_hz * duration_ms / 1000
 *
 * @param freq_hz Frequency in Hz (typical range 500-2000Hz)
 * @param duration_ms Duration in milliseconds
 *
 * @warning This is a blocking function that busy-waits for the duration.
 */
void buzzer_beep(uint16_t freq_hz, uint16_t duration_ms);

/**
 * @brief Play completion melody (3 beeps)
 *
 * @details Plays a distinctive melody to indicate winding complete:
 *          - 1000Hz for 150ms
 *          - 100ms pause
 *          - 1000Hz for 150ms
 *          - 100ms pause
 *          - 1500Hz for 300ms (higher pitch finale)
 *
 * @warning This is a blocking function (~900ms total duration).
 */
void buzzer_complete_melody(void);

#endif /* BUZZER_H */
