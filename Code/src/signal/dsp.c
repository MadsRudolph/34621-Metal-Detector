/*
 * dsp.c
 * DFT analyse til metal detektor
 *
 * - Optimeret DFT med 4x oversampling (ingen trig funktioner)
 * - Beregner magnitude og fase i grader
 */

#include "dsp.h"
#include "../app/debug.h"
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

/* Antal samples per DFT vindue */
#define N  64

/* DC offset til at fjerne (ADC midtpunkt) */
#define ADC_middelvaerdi 512

/*
 * DFT koefficienter ved 4x oversampling:
 * cos: [1,0,-1,0], sin: [0,1,0,-1]
 * Dette gør beregningen triviel - kun addition/subtraktion
 */

/* Intern sample tæller */
static uint8_t i = 0;

/* Akkumulatorer (signed for korrekt subtraktion) */
static int32_t Re = 0;
static int32_t Im = 0;

/* Buffere til main loop (fyldes når DFT er færdig) */
volatile int32_t Re_buff = 0;
volatile int32_t Im_buff = 0;
volatile uint8_t DFT_done = 0;

/* Resultater fra DFT_Calc (rå værdier) */
volatile uint16_t magnitude = 0;
volatile int16_t phase_deg = 0;

/* Filtrerede resultater (til display og detektor) */
volatile uint16_t magnitude_filt = 0;
volatile int16_t phase_filt = 0;

/*
 * IIR filter koefficient (fixed-point)
 * alpha = 32/256 = 0.125 (langsom respons, stabil)
 * Højere værdi = hurtigere respons, mere støj
 */
#define IIR_ALPHA 32

/*
 * DFT_sum
 * Kald fra ADC ISR for hver sample
 * Akkumulerer Re og Im ved brug af 4x oversampling trick
 */
void DFT_sum(uint16_t ADC_Raw)
{
    /* Log sample index til debug */
    debug_log_dft_index(i);

    /* Fjern DC offset, nu signed */
    int16_t xn = (int16_t)ADC_Raw - ADC_middelvaerdi;

    /* Anvend koefficient baseret på sample index mod 4 */
    switch(i & 3) {
        case 0: /* cos(0)*xn = 1*xn */
            Re += xn;
            break;
        case 1: /* -sin(pi/2)*xn = -1*xn */
            Im -= xn;
            break;
        case 2: /* cos(Pi)*xn = -1*xn */
            Re -= xn;
            break;
        case 3: /* -sin(3Pi/2)*xn = 1*xn */
            Im += xn;
            break;
    }

    i++;
    if (i >= N) {
        /* DFT vindue komplet - gem resultater */
        i = 0;
        DFT_done = 1;
        Re_buff = Re;
        Im_buff = Im;

        /* Log færdig DFT til debug */
        debug_log_dft_done(Re_buff, Im_buff);

        Re = 0;
        Im = 0;
    }
}

/* isqrt() er defineret i debug.c og deklareret i debug.h */

/*
 * iatan2
 * Hurtig heltal atan2 approksimation
 * Returnerer vinkel i grader (-180 til +180)
 */
static int16_t iatan2(int32_t y, int32_t x)
{
    /* Håndter special cases */
    if (x == 0 && y == 0) return 0;
    if (x == 0) return (y > 0) ? 90 : -90;
    if (y == 0) return (x > 0) ? 0 : 180;

    int16_t angle;
    int32_t abs_y = (y < 0) ? -y : y;
    int32_t abs_x = (x < 0) ? -x : x;

    /*
     * Approksimation: atan(y/x) ≈ 45 * y/x for små vinkler
     * z er skaleret 0-64 for at repræsentere 0-1
     */
    if (abs_x >= abs_y) {
        /* |vinkel| <= 45 grader */
        int32_t z = (abs_y * 64) / abs_x;
        angle = (z * 45) / 64;
    } else {
        /* |vinkel| > 45 grader */
        int32_t z = (abs_x * 64) / abs_y;
        angle = 90 - (z * 45) / 64;
    }

    /* Juster for kvadrant */
    if (x < 0) {
        angle = 180 - angle;
    }
    if (y < 0) {
        angle = -angle;
    }

    return angle;
}

/*
 * DFT_Calc
 * Beregn magnitude og fase fra Re_buff/Im_buff
 * Anvender IIR lavpasfilter for stabil output
 * Kald efter DFT_done == 1
 */
void DFT_Calc(void)
{
    int32_t re = Re_buff;
    int32_t im = Im_buff;

    /*
     * Magnitude = sqrt(Re^2 + Im^2)
     * Skaler ned for at undgå overflow: divider med N/4 = 16
     */
    int32_t re_scaled = re / 16;
    int32_t im_scaled = im / 16;

    uint32_t mag_squared = (uint32_t)(re_scaled * re_scaled + im_scaled * im_scaled);
    magnitude = (uint16_t)isqrt(mag_squared);

    /* Fase = atan2(Im, Re) i grader */
    phase_deg = iatan2(im, re);

    /*
     * IIR lavpasfilter: y[n] = α*x[n] + (1-α)*y[n-1]
     * Fixed-point: y = (α*x + (256-α)*y) / 256
     */
    magnitude_filt = (uint16_t)(
        ((uint32_t)IIR_ALPHA * magnitude +
         (uint32_t)(256 - IIR_ALPHA) * magnitude_filt) / 256
    );

    phase_filt = (int16_t)(
        ((int32_t)IIR_ALPHA * phase_deg +
         (int32_t)(256 - IIR_ALPHA) * phase_filt) / 256
    );
}
