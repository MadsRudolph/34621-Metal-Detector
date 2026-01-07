/*
 * detector.h
 * Metal detektion og klassifikation
 *
 * Fasebaseret klassifikation:
 * - Fase <= 65° = Ferromagnetisk (jern, stål)
 * - Fase > 65° = Ikke-ferromagnetisk (kobber, guld, aluminium)
 */

#ifndef DETECTOR_H
#define DETECTOR_H

#include <stdint.h>

/* Metal typer */
typedef enum {
    METAL_NONE,         /* Intet metal detekteret */
    METAL_FERROUS,      /* Jern, stål, nikkel */
    METAL_NON_FERROUS   /* Kobber, guld, sølv, aluminium */
} metal_type_t;

/* Detektor tilstand */
typedef struct {
    uint16_t baseline_mag;    /* Kalibreret magnitude (ingen metal) */
    int16_t baseline_phase;   /* Kalibreret fase */
    uint8_t calibrated;       /* 1 = kalibreret, 0 = ikke kalibreret */
} detector_state_t;

/* Global detektor tilstand */
extern detector_state_t detector;

/* Initialiser detektor (nulstil kalibrering) */
void detector_init(void);

/* Kalibrer baseline (kald uden metal over spole) */
void detector_calibrate(void);

/* Analyser og klassificer metal baseret på DFT resultater */
metal_type_t detector_classify(void);

/* Hent signal styrke (0-100) relativt til baseline */
uint8_t detector_get_strength(void);

#endif
