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
 *   2. Positivt faseskift → Ferro (jernholdigt)
 *   3. Negativt faseskift → Non-Ferro (kobber, aluminium)
 *
 * Fysik baggrund:
 *   - Ferromagnetiske metaller (jern, stål) øger induktans
 *     der giver positivt faseskift
 *   - Non-ferro metaller (kobber, aluminium) skaber hvirvelstrømme
 *     der modvirker feltet og giver negativt faseskift
 */
uint8_t classify_metal(void) {
    // Kan ikke klassificere uden kalibrering
    if (!is_calibrated) {
        return METAL_NONE;
    }

    // Beregn faseforskel fra baseline
    int16_t phase_diff = ang_filtered - cal_ang;

    // Tjek om metal er detekteret (magnitude øget med mindst 20% eller +3)
    int32_t mag_delta = (int32_t)mag_filtered - (int32_t)cal_mag;
    int16_t mag_threshold = cal_mag / 5;  // 20% af baseline
    if (mag_threshold < 3) mag_threshold = 3;  // Minimum threshold
    if (mag_delta <= mag_threshold) {
        return METAL_NONE;
    }

    // Ferro: positivt faseskift (øget induktans)
    if (phase_diff >= 0) {
        return METAL_FERRO;
    }

    // Non-ferro: negativt faseskift (hvirvelstrømme)
    return METAL_NONFERRO;
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
