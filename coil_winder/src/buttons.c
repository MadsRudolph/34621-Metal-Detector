/**
 * @file buttons.c
 * @brief Button input handling with debouncing
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of debounced button polling using edge detection.
 *          Buttons are active-low: HIGH = released, LOW = pressed.
 */

#include "buttons.h"
#include "config.h"
#include "timer.h"
#include <avr/io.h>

/**
 * @brief Previous button state for edge detection
 * @details Initialized to 0xFF (all released) since buttons have pull-ups.
 */
static uint8_t g_btn_prev = 0xFF;

/**
 * @brief Timestamp of last button event for debounce
 */
static uint32_t g_last_time = 0;

void buttons_init(void)
{
    /* Configure PD3, PD4, PD5 as inputs (clear DDR bits) */
    BTN_DDR &= ~((1 << BTN_RESET_PIN) | (1 << BTN_PRESET_PIN) | (1 << BTN_START_PIN));

    /* Enable internal pull-ups (set PORT bits) */
    BTN_PORT |= (1 << BTN_RESET_PIN) | (1 << BTN_PRESET_PIN) | (1 << BTN_START_PIN);
}

uint8_t buttons_poll(void)
{
    uint8_t events = 0;

    /* Read current button state (masked to only our pins) */
    uint8_t btn_state = BTN_PINR & ((1 << BTN_RESET_PIN) | (1 << BTN_PRESET_PIN) | (1 << BTN_START_PIN));
    uint32_t now = millis();

    /*
     * Debounce check:
     * If we're within the debounce window, don't report events
     * but still update the previous state to track releases.
     * This prevents the button from appearing "stuck" if released
     * during the debounce period.
     */
    if (now - g_last_time < DEBOUNCE_MS) {
        g_btn_prev = btn_state;
        return 0;
    }

    /*
     * Detect falling edge: was HIGH (released), now LOW (pressed)
     * Active-low buttons: bit set = released, bit clear = pressed
     */

    /* Reset button (PD3) */
    if ((g_btn_prev & (1 << BTN_RESET_PIN)) && !(btn_state & (1 << BTN_RESET_PIN))) {
        events |= BTN_RESET_PRESSED;
    }

    /* Preset button (PD4) */
    if ((g_btn_prev & (1 << BTN_PRESET_PIN)) && !(btn_state & (1 << BTN_PRESET_PIN))) {
        events |= BTN_PRESET_PRESSED;
    }

    /* Start button (PD5) */
    if ((g_btn_prev & (1 << BTN_START_PIN)) && !(btn_state & (1 << BTN_START_PIN))) {
        events |= BTN_START_PRESSED;
    }

    /* Update state for next comparison */
    g_btn_prev = btn_state;

    /* Reset debounce timer on any button press */
    if (events) {
        g_last_time = now;
    }

    return events;
}
