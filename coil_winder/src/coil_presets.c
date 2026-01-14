/**
 * @file coil_presets.c
 * @brief Coil preset definitions and winder state management
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of preset table and state management functions.
 */

#include "coil_presets.h"
#include "encoder.h"
#include "servo.h"

/**
 * @brief Preset table with VLF metal detector coil specifications
 *
 * @details Each entry defines:
 *          - name: Short identifier for display
 *          - total_turns: Complete turns required
 *          - layers: Number of winding layers
 *          - winding_width: Traverse width in mm
 *          - wire_diameter: Wire gauge in mm * 100
 */
const coil_preset_t coil_presets[] = {
    { "TX",   68,  2, 18, 52 },   /* TX: 68 turns, 2 layers, 18mm, 0.52mm wire */
    { "RX",   170, 4, 14, 32 },   /* RX: 170 turns, 4 layers, 14mm, 0.32mm wire */
    { "BUCK", 35,  1, 18, 52 }    /* Bucking: 35 turns, 1 layer, 18mm, 0.52mm wire */
};

/** @brief Number of presets in the table */
const uint8_t NUM_PRESETS = sizeof(coil_presets) / sizeof(coil_presets[0]);

const coil_preset_t* preset_get_current(winder_context_t *ctx)
{
    return &coil_presets[ctx->current_preset];
}

void preset_next(winder_context_t *ctx)
{
    ctx->current_preset = (ctx->current_preset + 1) % NUM_PRESETS;
}

void winder_reset(winder_context_t *ctx)
{
    /* Reset turn counter */
    encoder_reset();

    /* Reset winder state */
    ctx->current_layer = 1;
    ctx->position_x100 = 0;
    ctx->traverse_direction = true;
    ctx->state = STATE_IDLE;

    /* Move servo to start position */
    const coil_preset_t *preset = preset_get_current(ctx);
    servo_set_position_mm(0, preset->winding_width);
}

const char* state_to_string(winder_state_t state)
{
    switch (state) {
        case STATE_IDLE:     return "IDLE";
        case STATE_WINDING:  return "RUN";
        case STATE_PAUSED:   return "PAUS";
        case STATE_COMPLETE: return "DONE";
        default:             return "???";
    }
}
