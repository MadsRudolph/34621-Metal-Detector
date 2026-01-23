/*
 * detection.h - Metal detektion og kalibrering
 *
 * Håndterer:
 *   - Start/Stop styring (Krav 9a)
 *   - Kalibrering (Krav 9b)
 *   - Metal klassificering: Ferro vs Non-Ferro (Krav 2)
 */

#ifndef DETECTION_H
#define DETECTION_H

#include <stdint.h>

/* --- Metal typer --- */
#define METAL_NONE      0   // Intet metal detekteret
#define METAL_FERRO     1   // Jernholdigt metal (stort negativt faseskift)
#define METAL_NONFERRO  2   // Ikke-jernholdigt metal (lille/positivt faseskift)

/* --- Globale variable --- */
// Detektor tilstand
extern uint8_t detection_active;    // 1=kører, 0=pauset

// Kalibrering tilstand
extern uint8_t is_calibrated;       // 1=kalibreret, 0=ikke kalibreret
extern uint16_t cal_mag;            // Kalibreret magnitude baseline
extern int16_t cal_ang;             // Kalibreret fase baseline

/* --- Funktioner --- */

/*
 * Klassificer detekteret metal baseret på faseskift
 *
 * Returnerer:
 *   METAL_NONE     - Intet metal (eller ikke kalibreret)
 *   METAL_FERRO    - Jernholdigt metal (stort negativt faseskift)
 *   METAL_NONFERRO - Ikke-jernholdigt metal
 */
uint8_t classify_metal(void);

/*
 * Udfør kalibrering - gem nuværende værdier som reference
 */
void calibrate(void);

/*
 * Toggle detektor aktiv/pauset
 */
void toggle_detection(void);

#endif /* DETECTION_H */
