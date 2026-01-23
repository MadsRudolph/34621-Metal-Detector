/*
 * dft.c - DFT beregning for Metal Detektor
 *
 * Implementerer en optimeret Goertzel-lignende DFT ved 2kHz
 * Udnytter at samples er periodiske med Pi/2 faseskift
 */

#include <math.h>
#include "include/dft.h"
#include "include/config.h"
#include "include/timer.h"

/* --- Globale variable --- */
// DFT akkumulator variable
volatile uint8_t j = 0;             // Sample tæller inden for vindue
volatile int32_t Re = 0;            // Real akkumulator
volatile int32_t Im = 0;            // Imaginær akkumulator

// DFT buffer (kopieres når vindue er komplet)
volatile int32_t Re_buff = 0;
volatile int32_t Im_buff = 0;

// Beregnede resultater
volatile uint16_t mag = 0;          // Magnitude
volatile int16_t ang = 0;           // Fase i grader (-180 til +180)

// DFT færdig flag
volatile uint8_t DFT_done = 0;

// Lokal variabel til transformeret sample
static volatile int16_t xn = 0;

/* --- DFT akkumulering --- */
/*
 * DFT_sum - Akkumulerer samples til DFT
 *
 * Denne funktion kaldes fra ADC ISR med 8kHz rate.
 * Ved at sample med 4x signalfrekvensen (8kHz / 2kHz = 4) får vi
 * samples med præcis Pi/2 (90°) faseskift mellem hver.
 *
 * Dette giver en elegant optimering:
 *   Sample 0: cos(0) = 1,    sin(0) = 0      → kun Real
 *   Sample 1: cos(π/2) = 0,  sin(π/2) = 1    → kun Imaginær
 *   Sample 2: cos(π) = -1,   sin(π) = 0      → kun Real (negativ)
 *   Sample 3: cos(3π/2) = 0, sin(3π/2) = -1  → kun Imaginær (negativ)
 *
 * Vi undgår dermed alle multiplikationer med 0 og reducerer beregningen
 * til simple additioner og subtraktioner.
 */
void DFT_sum(int16_t ADC_Raw) {
    // Fjern DC offset
    xn = ADC_Raw - ADC_MIDDELVAERDI;

    // Akkumuler til Real og Imaginær del baseret på sample position
    // (j & 3) giver værdier 0,1,2,3,0,1,2,3,... uanset j's størrelse
    switch (j & 3) {
        case 0: // cos(0)*xn = 1*xn
            Re += RePhase1 * xn;
            break;

        case 1: // -sin(π/2)*xn = -1*xn (negativ da DFT definition)
            Im += -ImPhase2 * xn;
            break;

        case 2: // cos(π)*xn = -1*xn
            Re += RePhase3 * xn;
            break;

        case 3: // -sin(3π/2)*xn = -(-1)*xn = xn
            Im += -ImPhase4 * xn;
            break;
    }
    j++;

    // Når vi har akkumuleret N samples, gem resultat og nulstil
    if (j >= N) {
        j = 0;

        // Overfør til buffer inden reset (ellers mister vi data)
        Re_buff = Re;
        Im_buff = Im;

        // Signaler at DFT er klar til beregning
        DFT_done = 1;

        // Nulstil akkumulatorer til næste vindue
        Re = 0;
        Im = 0;

        // Reset sync flag - venter på næste TX rising edge
        rising_edge_Flag = 0;
    }
}

/* --- DFT beregning --- */
/*
 * DFT_Calc - Beregner magnitude og fase
 *
 * Magnitude = sqrt(Re² + Im²) / N
 * Fase = atan2(Im, Re) * (180/π)
 */
void DFT_Calc(void) {
    // Magnitude normaliseret med antal samples
    mag = sqrt((float)Re_buff * Re_buff + (float)Im_buff * Im_buff) / N;

    // Fase i grader (57.2957795131 = 180/π)
    ang = atan2((float)Im_buff, (float)Re_buff) * 57.2957795131;
}
