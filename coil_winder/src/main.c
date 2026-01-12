/**
 * @file main.c
 * @brief Coil Winder Controller - DTU 34621 Metal Detector Project
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Main application for the coil winder controller.
 *          Coordinates all peripherals to automate coil winding:
 *          - Counts turns via ITR8307 optical encoder
 *          - Controls wire guide servo for layer traverse
 *          - Displays status on SSD1306 OLED
 *          - Handles button input for control
 *          - Plays audio feedback on completion
 *
 *          Hardware: ATmega328P @ 16MHz (Arduino UNO)
 *
 * @note Add 100uF capacitor on servo power and 100nF on sensor
 *       to filter PWM noise from servo motor.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "timer.h"
#include "encoder.h"
#include "servo.h"
#include "buttons.h"
#include "buzzer.h"
#include "twi.h"
#include "ssd1306.h"
#include "coil_presets.h"

/**
 * @brief Global winder context instance
 * @details Holds all runtime state for the winding operation.
 */
static winder_context_t g_winder = {
    .current_preset = 0,
    .current_layer = 1,
    .state = STATE_IDLE,
    .position_x100 = 0,
    .traverse_direction = true
};

/**
 * @brief Process a detected turn
 *
 * @details Called when encoder_check_turn() returns true during winding.
 *          Updates traverse position, handles layer changes, and checks
 *          for winding completion.
 *
 *          Traverse algorithm:
 *          1. Move position by wire diameter
 *          2. If at boundary, reverse direction and increment layer
 *          3. Update servo position
 *          4. Check if total turns reached
 */
static void process_turn(void)
{
    const coil_preset_t *preset = preset_get_current(&g_winder);

    /* Move traverse by wire diameter (in mm * 100) */
    int16_t step = preset->wire_diameter;

    if (g_winder.traverse_direction) {
        g_winder.position_x100 += step;
    } else {
        g_winder.position_x100 -= step;
    }

    /* Check boundaries and handle layer change */
    int16_t max_pos = (int16_t)preset->winding_width * 100;

    if (g_winder.position_x100 >= max_pos) {
        /* Hit right boundary - reverse direction */
        g_winder.position_x100 = max_pos;
        g_winder.traverse_direction = false;
        g_winder.current_layer++;
    } else if (g_winder.position_x100 <= 0) {
        /* Hit left boundary - reverse direction */
        g_winder.position_x100 = 0;
        g_winder.traverse_direction = true;
        g_winder.current_layer++;
    }

    /* Update servo to new position */
    servo_set_position_mm(g_winder.position_x100, preset->winding_width);

    /* Check for winding completion */
    if (encoder_get_count() >= preset->total_turns) {
        g_winder.state = STATE_COMPLETE;
        buzzer_complete_melody();
    }
}

/**
 * @brief Update OLED display with current status
 *
 * @details Draws the winder status screen at 10Hz refresh rate.
 *          Display layout:
 *          - Row 0: Preset name, width, state
 *          - Row 14: Large turn counter, target, layer
 *          - Row 42: Direction indicator, position
 *          - Row 52: Progress bar with percentage
 */
static void update_display(void)
{
    const coil_preset_t *preset = preset_get_current(&g_winder);
    uint16_t turns = encoder_get_count();

    ssd1306_clear();

    /* Header: Preset name and winding width */
    ssd1306_draw_string(0, 0, preset->name, 1);
    ssd1306_draw_int(30, 0, preset->winding_width, 2, 1);
    ssd1306_draw_string(42, 0, "mm", 1);

    /* State indicator (right side) */
    ssd1306_draw_string(98, 0, state_to_string(g_winder.state), 1);

    /* Turn count (large, scale 3) */
    ssd1306_draw_int(0, 14, turns, 3, 3);

    /* Target turns (small) */
    ssd1306_draw_string(60, 14, "/", 1);
    ssd1306_draw_int(66, 14, preset->total_turns, 3, 1);

    /* Layer counter */
    ssd1306_draw_string(60, 26, "L", 1);
    ssd1306_draw_int(66, 26, g_winder.current_layer, 1, 1);
    ssd1306_draw_string(72, 26, "/", 1);
    ssd1306_draw_int(78, 26, preset->layers, 1, 1);

    /* Direction indicator and position */
    ssd1306_draw_string(0, 42, g_winder.traverse_direction ? ">>>" : "<<<", 1);
    ssd1306_draw_int(24, 42, g_winder.position_x100 / 100, 2, 1);
    ssd1306_draw_string(36, 42, ".", 1);
    ssd1306_draw_int(42, 42, (g_winder.position_x100 % 100) / 10, 1, 1);
    ssd1306_draw_string(48, 42, "/", 1);
    ssd1306_draw_int(54, 42, preset->winding_width, 2, 1);
    ssd1306_draw_string(66, 42, "mm", 1);

    /* Progress bar */
    ssd1306_draw_progress(0, 52, 100, 10, turns, preset->total_turns);

    /* Percentage complete */
    uint8_t pct = ((uint32_t)turns * 100) / preset->total_turns;
    ssd1306_draw_int(105, 54, pct, 3, 1);
    ssd1306_draw_string(123, 54, "%", 1);

    ssd1306_update();
}

/**
 * @brief Handle button press events
 *
 * @details Processes button events from buttons_poll():
 *          - Reset: Always resets winder to idle state
 *          - Preset: Cycles preset (only in IDLE or COMPLETE state)
 *          - Start: Toggles winding/paused, or resets from COMPLETE
 */
static void handle_buttons(void)
{
    uint8_t events = buttons_poll();

    /* Reset button - always active */
    if (events & BTN_RESET_PRESSED) {
        winder_reset(&g_winder);
    }

    /* Preset button - only when idle or complete */
    if (events & BTN_PRESET_PRESSED) {
        if (g_winder.state == STATE_IDLE || g_winder.state == STATE_COMPLETE) {
            preset_next(&g_winder);
            winder_reset(&g_winder);
        }
    }

    /* Start/Stop button - state-dependent action */
    if (events & BTN_START_PRESSED) {
        if (g_winder.state == STATE_IDLE || g_winder.state == STATE_PAUSED) {
            /* Start or resume winding */
            g_winder.state = STATE_WINDING;
        } else if (g_winder.state == STATE_WINDING) {
            /* Pause winding */
            g_winder.state = STATE_PAUSED;
        } else if (g_winder.state == STATE_COMPLETE) {
            /* Reset after completion */
            winder_reset(&g_winder);
        }
    }
}

/**
 * @brief Main entry point
 *
 * @details Initializes all peripherals and runs the main loop.
 *          The main loop:
 *          1. Polls buttons for user input
 *          2. Processes encoder turns (with debounce) when winding
 *          3. Updates display at 10Hz (100ms interval)
 *
 * @return int Never returns (infinite loop)
 */
int main(void)
{
    /* Initialize all peripherals */
    timer_init();       /* Millisecond timer (Timer0) */
    twi_init();         /* I2C for OLED */
    encoder_init();     /* Turn counter (INT0) */
    servo_init();       /* Wire guide (Timer1 PWM) */
    buttons_init();     /* User input */
    buzzer_init();      /* Audio feedback */
    ssd1306_init();     /* OLED display */

    /* Enable global interrupts */
    sei();

    /* Reset to initial state */
    winder_reset(&g_winder);

    /* Startup beep to indicate ready */
    buzzer_beep(1000, 100);

    /* Display update timing */
    uint32_t last_display = 0;

    /* Main loop */
    while (1) {
        /* Handle button input */
        handle_buttons();

        /* Process turns when winding (with debounce filtering) */
        if (encoder_check_turn() && g_winder.state == STATE_WINDING) {
            process_turn();
        }

        /* Update display at 10Hz (100ms interval) */
        if (millis() - last_display >= DISPLAY_UPDATE_MS) {
            last_display = millis();
            update_display();
        }
    }

    return 0;
}
