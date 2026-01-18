/*
 * config.h - Konfiguration for Metal Detektor
 *
 * Alle globale indstillinger og konstanter samlet ét sted
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ============ DFT INDSTILLINGER ============ */
#define F_SAMPLE 8000       // Sample frekvens (Hz) - skal være 4x signal frekvens
#define F_SIGNAL 2000       // TX/RX signal frekvens (Hz)
#define N 64                // Antal samples per DFT vindue (flere = mere præcist, men langsommere)

/* ============ DFT FASEKOEFFICIENTER ============ */
// For samples periodisk Pi/2 ved vælgelse af k = 16
// Dette sikrer at vi ikke behøver at lave udregninger med Realdel og Imaginærdel samtidig
// og ydermere betyder dette at vi kan forudregne og slippe for at regne med andet end +/- 1
#define RePhase1  1         // cos(0)
#define ImPhase2  1         // sin(Pi/2)
#define RePhase3 -1         // cos(Pi)
#define ImPhase4 -1         // sin(3Pi/2)

/* ============ ADC INDSTILLINGER ============ */
#define ADC_MIDDELVAERDI 512  // DC offset kompensation (512 = midt i 10-bit range)

/* ============ METAL KLASSIFICERING ============ */
#define PHASE_THRESHOLD 15  // Grader - tærskel for ferro/non-ferro klassificering

/* ============ PIN KONFIGURATION ============ */
// TX signal output
#define TX_PIN    PB1       // Pin 9 = TX signal (2kHz firkantbølge til sendespole)

// Knapper (active low med pull-up)
#define BTN_START_STOP  PD2 // Start/Stop detektor (Krav 9a)
#define BTN_CALIBRATE   PD3 // Kalibrering (Krav 9b)
#define BTN_DEBUG       PD4 // Skift mellem DFT og Debug skærm

// Buzzer output (PWM via Timer2)
#define BUZZER_PIN      PB3 // Pin 11 = Buzzer (OC2A)

#endif /* CONFIG_H */
