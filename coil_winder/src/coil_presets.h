/**
 * @file coil_presets.h
 * @brief Coil preset definitions and winder state machine
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Defines coil winding parameters for different detector coils:
 *          - TX: Transmit coil (68 turns, 2 layers, 0.52mm wire)
 *          - RX: Receive coil (170 turns, 4 layers, 0.32mm wire)
 *          - BUCK: Bucking coil (35 turns, 1 layer, 0.52mm wire)
 *
 *          Also provides winder state machine and context management.
 */

#ifndef COIL_PRESETS_H
#define COIL_PRESETS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup CoilPresets Coil Preset Definitions
 * @brief Winding parameters for each coil type
 * @{
 */

/**
 * @brief Coil winding preset parameters
 */
typedef struct {
    const char *name;       /**< Short display name (max 4 chars) */
    uint16_t total_turns;   /**< Total number of turns to wind */
    uint8_t layers;         /**< Number of winding layers */
    uint8_t winding_width;  /**< Winding width in mm */
    uint8_t wire_diameter;  /**< Wire diameter in mm * 100 (e.g., 52 = 0.52mm) */
} coil_preset_t;

/** @} */

/**
 * @defgroup WinderState Winder State Machine
 * @brief State definitions for the winding process
 * @{
 */

/**
 * @brief Winder operating states
 */
typedef enum {
    STATE_IDLE,     /**< Waiting to start, preset can be changed */
    STATE_WINDING,  /**< Actively winding, counting turns */
    STATE_PAUSED,   /**< Winding paused, can resume or reset */
    STATE_COMPLETE  /**< Target turns reached, winding complete */
} winder_state_t;

/** @} */

/**
 * @defgroup WinderContext Winder Context
 * @brief Runtime state variables for the winder
 * @{
 */

/**
 * @brief Winder runtime context
 */
typedef struct {
    uint8_t current_preset;     /**< Index into coil_presets[] */
    uint8_t current_layer;      /**< Current winding layer (1-based) */
    winder_state_t state;       /**< Current operating state */
    int16_t position_x100;      /**< Traverse position in mm * 100 */
    bool traverse_direction;    /**< true = forward, false = reverse */
} winder_context_t;

/** @} */

/**
 * @defgroup PresetTable External Preset Table
 * @brief Global preset definitions
 * @{
 */

/** @brief Array of available coil presets */
extern const coil_preset_t coil_presets[];

/** @brief Number of available presets */
extern const uint8_t NUM_PRESETS;

/** @} */

/**
 * @defgroup PresetFunctions Preset and State Functions
 * @brief Functions for preset and state management
 * @{
 */

/**
 * @brief Get current preset
 *
 * @param ctx Pointer to winder context
 * @return Pointer to current coil_preset_t (read-only)
 */
const coil_preset_t* preset_get_current(winder_context_t *ctx);

/**
 * @brief Cycle to next preset
 *
 * @details Increments preset index with wraparound.
 *          Does not reset winder state (call winder_reset separately).
 *
 * @param ctx Pointer to winder context
 */
void preset_next(winder_context_t *ctx);

/**
 * @brief Reset winder to initial state
 *
 * @details Resets all winder state for starting a new winding:
 *          - Clears turn counter
 *          - Resets to layer 1
 *          - Sets traverse position to 0
 *          - Sets direction to forward
 *          - Sets state to IDLE
 *          - Moves servo to start position
 *
 * @param ctx Pointer to winder context
 */
void winder_reset(winder_context_t *ctx);

/**
 * @brief Get state string for display
 *
 * @details Returns a 4-character string representation of the state
 *          for display on the OLED.
 *
 * @param state Current winder state
 * @return String: "IDLE", "RUN", "PAUS", or "DONE"
 */
const char* state_to_string(winder_state_t state);

/** @} */

#endif /* COIL_PRESETS_H */
