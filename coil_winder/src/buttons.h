/**
 * @file buttons.h
 * @brief Button input handling with debouncing
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Provides debounced button input for three pushbuttons:
 *          - Reset: Resets turn counter and winder state
 *          - Preset: Cycles through coil presets (TX/RX/BUCK)
 *          - Start/Stop: Toggles winding operation
 *
 *          Buttons are active-low with internal pull-ups enabled.
 *          50ms debounce time filters mechanical bounce.
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup ButtonEvents Button Event Flags
 * @brief Bitmask flags returned by buttons_poll()
 * @{
 */

/** @brief Reset button was pressed (PD3/D3) */
#define BTN_RESET_PRESSED   (1 << 0)

/** @brief Preset cycle button was pressed (PD4/D4) */
#define BTN_PRESET_PRESSED  (1 << 1)

/** @brief Start/Stop button was pressed (PD5/D5) */
#define BTN_START_PRESSED   (1 << 2)

/** @} */

/**
 * @brief Initialize button inputs with pull-ups
 *
 * @details Configures PD3, PD4, PD5 as inputs with internal pull-ups.
 *          Buttons connect pin to GND when pressed (active-low).
 */
void buttons_init(void);

/**
 * @brief Poll buttons and return pressed events
 *
 * @details Detects falling edges (button press) with 50ms debounce.
 *          Call this function regularly from the main loop.
 *
 *          Debounce logic:
 *          - Tracks previous button state
 *          - Ignores state changes within 50ms of last event
 *          - Returns event flags only on valid falling edge
 *
 * @return uint8_t Bitmask of BTN_*_PRESSED flags for buttons pressed
 *                 since last call. Returns 0 if no buttons pressed.
 *
 * @note State tracking continues during debounce period to avoid
 *       missing button releases that would cause stuck state.
 */
uint8_t buttons_poll(void);

#endif /* BUTTONS_H */
