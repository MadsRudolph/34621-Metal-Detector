/*
 * detector.c
 * Metal klassifikation og kalibrering
 *
 * Bruger fase til at skelne mellem metaltyper:
 * - Ferromagnetiske metaller: fase <= 65°
 * - Ikke-ferromagnetiske metaller: fase > 65°
 */

#include "detector.h"
#include "../signal/dsp.h"

/* Fase tærskel for klassifikation (grader) */
#define PHASE_THRESHOLD 65

/* Hysterese for at undgå hurtig skift (grader) */
#define PHASE_HYSTERESIS 10

/* Minimum magnitude for detektion */
#define MIN_DETECTION_MAG 20

/* Sidste klassifikation (til hysterese) */
static metal_type_t last_type = METAL_NONE;

/* Global detektor tilstand */
detector_state_t detector;

/*
 * detector_init
 * Nulstil detektor til ukendt tilstand
 */
void detector_init(void)
{
    detector.baseline_mag = 0;
    detector.baseline_phase = 0;
    detector.calibrated = 0;
}

/*
 * detector_calibrate
 * Gem nuværende filtrerede magnitude/fase som baseline
 * Kald når der ikke er metal over spolen
 */
void detector_calibrate(void)
{
    detector.baseline_mag = magnitude_filt;
    detector.baseline_phase = phase_filt;
    detector.calibrated = 1;
}

/*
 * detector_classify
 * Klassificer metal baseret på fase med hysterese
 * Returnerer METAL_NONE hvis signal er for svagt
 */
metal_type_t detector_classify(void)
{
    /* Tjek om kalibreret */
    if (!detector.calibrated) {
        last_type = METAL_NONE;
        return METAL_NONE;
    }

    /*
     * Beregn magnitude ændring fra baseline (brug filtreret)
     * Kun positiv ændring tæller (metal øger signal)
     */
    int16_t mag_diff;
    if (magnitude_filt > detector.baseline_mag) {
        mag_diff = magnitude_filt - detector.baseline_mag;
    } else {
        /* Signal under baseline = ingen detektion */
        last_type = METAL_NONE;
        return METAL_NONE;
    }

    /* Tjek om signal er stærkt nok til detektion */
    if (mag_diff < MIN_DETECTION_MAG) {
        last_type = METAL_NONE;
        return METAL_NONE;
    }

    /*
     * Klassificer med hysterese (brug filtreret fase):
     * - Skift til FERRO kun hvis fase < (threshold - hysterese)
     * - Skift til NON_FE kun hvis fase > (threshold + hysterese)
     * - Ellers behold sidste type
     */
    if (phase_filt <= (PHASE_THRESHOLD - PHASE_HYSTERESIS)) {
        last_type = METAL_FERROUS;
    } else if (phase_filt >= (PHASE_THRESHOLD + PHASE_HYSTERESIS)) {
        last_type = METAL_NON_FERROUS;
    }
    /* Hvis fase er mellem 55-75°, behold sidste klassifikation */

    return last_type;
}

/*
 * detector_get_strength
 * Returner signal styrke som 0-100
 * Kun positiv afvigelse fra baseline giver styrke
 */
uint8_t detector_get_strength(void)
{
    if (!detector.calibrated) {
        return 0;
    }

    /* Kun positiv afvigelse tæller */
    if (magnitude_filt <= detector.baseline_mag) {
        return 0;
    }

    uint16_t mag_diff = magnitude_filt - detector.baseline_mag;

    /* Skaler til 0-100 (antag max afvigelse ~500) */
    uint16_t strength = (mag_diff * 100) / 500;

    if (strength > 100) {
        strength = 100;
    }

    return (uint8_t)strength;
}
