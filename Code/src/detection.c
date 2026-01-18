/*
 * detection.c - Metal detektion og kalibrering
 *
 * Håndterer:
 *   - Start/Stop styring (Krav 9a)
 *   - Kalibrering (Krav 9b)
 *   - Metal klassificering: Ferro vs Non-Ferro (Krav 2)
 */

#include "detection.h"
#include "filter.h"
#include "config.h"

/* ============ GLOBALE VARIABLE ============ */
// Detektor tilstand
uint8_t detection_active = 1;       // Start i aktiv tilstand

// Kalibrering
uint8_t is_calibrated = 0;          // Ikke kalibreret ved opstart
uint16_t cal_mag = 0;               // Kalibreret magnitude baseline
int16_t cal_ang = 0;                // Kalibreret fase baseline

/* ============ METAL KLASSIFICERING ============ */
/*
 * classify_metal - Klassificer metal type baseret på faseskift
 *
 * Metode:
 *   1. Beregn faseforskel fra kalibreret baseline
 *   2. Stort negativt faseskift → Ferro (jernholdigt)
 *   3. Lille/positivt faseskift med øget magnitude → Non-Ferro
 *
 * Fysik baggrund:
 *   - Ferromagnetiske metaller (jern, stål) skaber modsat rettet felt
 *     der giver negativt faseskift
 *   - Non-ferro metaller (kobber, aluminium) skaber hvirvelstrømme
 *     der giver lille eller positivt faseskift
 */
uint8_t classify_metal(void) {
    // Kan ikke klassificere uden kalibrering
    if (!is_calibrated) {
        return METAL_NONE;
    }

    // Beregn faseforskel fra baseline
    int16_t phase_diff = ang_filtered - cal_ang;

    // Ferro: stort negativt faseskift
    if (phase_diff < -PHASE_THRESHOLD) {
        return METAL_FERRO;
    }

    // Non-ferro: magnitude øget væsentligt (metal detekteret)
    // men faseskift er lille eller positivt
    if (mag_filtered > cal_mag + 20) {
        return METAL_NONFERRO;
    }

    // Intet metal detekteret
    return METAL_NONE;
}

/* ============ KALIBRERING ============ */
void calibrate(void) {
    cal_mag = mag_filtered;     // Gem nuværende magnitude som reference
    cal_ang = ang_filtered;     // Gem nuværende fase som reference
    is_calibrated = 1;
}

/* ============ START/STOP ============ */
void toggle_detection(void) {
    detection_active = !detection_active;
}
