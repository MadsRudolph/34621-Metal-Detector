/*
 * dsp.c
 * DFT analysis + IIR filtering
 *
 * - Optimized DFT using 4x oversampling (no trig needed)
 * - IIR low-pass for display smoothing
 * - Integer math only (no floating-point for AVR performance)
 */

#include "dsp.h"
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>

#define Fs 8000
#define N  64
#define f0 2000
#define ADC_middelvaerdi 512

/* Phase coefficients for 4x oversampling at Fs/4 */
#define RePhase1  1  // cos(0)
#define ImPhase2  1  // sin(Pi/2)
#define RePhase3 -1  // cos(Pi)
#define ImPhase4 -1  // sin(3Pi/2)

static uint8_t i = 0;

/* DFT accumulators - signed to handle subtraction correctly */
volatile int32_t Re = 0;
volatile int32_t Re_buff = 0;
volatile int32_t Im = 0;
volatile int32_t Im_buff = 0;

/* Sample value - signed for DC offset removal */
volatile int16_t xn = 0;

volatile uint8_t DFT_done;

/* Exported results for main.c */
volatile uint16_t magnitude = 0;
volatile int16_t phase_deg = 0;
volatile uint16_t magnitude_filt = 0;
volatile int16_t phase_filt = 0;

/* IIR filter coefficient: alpha = 32/256 = 0.125 */
#define IIR_ALPHA 32

/*
 * Integer square root (Newton-Raphson)
 * Much faster than floating-point sqrt() on AVR
 */
static uint16_t isqrt(uint32_t n) {
    if (n == 0) return 0;

    uint32_t x = n;
    uint32_t y = (x + 1) / 2;

    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return (uint16_t)x;
}

/*
 * Fast integer atan2 approximation
 * Returns angle in degrees (-180 to +180)
 */
static int16_t fast_atan2_deg(int32_t y, int32_t x) {
    if (x == 0 && y == 0) return 0;
    if (x == 0) return (y > 0) ? 90 : -90;
    if (y == 0) return (x > 0) ? 0 : 180;

    int16_t angle;
    int32_t abs_y = (y < 0) ? -y : y;
    int32_t abs_x = (x < 0) ? -x : x;

    if (abs_x >= abs_y) {
        int32_t z = (abs_y * 64) / abs_x;
        angle = (z * 45) / 64;
    } else {
        int32_t z = (abs_x * 64) / abs_y;
        angle = 90 - (z * 45) / 64;
    }

    if (x < 0) angle = 180 - angle;
    if (y < 0) angle = -angle;

    return angle;
}

/*
 * DFT_sum() - Called from ADC ISR for each sample
 * Accumulates DFT over 64 samples using 4x oversampling
 * Sets DFT_done = 1 when window is complete
 */
void DFT_sum(int16_t ADC_Raw) {
    xn = ADC_Raw - ADC_middelvaerdi;

    switch (i & 3) {
        case 0:  // cos(0)*xn = 1*xn
            Re += RePhase1 * xn;
            break;
        case 1:  // -sin(pi/2)*xn = -1*xn
            Im += -ImPhase2 * xn;
            break;
        case 2:  // cos(Pi)*xn = -1*xn
            Re += RePhase3 * xn;
            break;
        case 3:  // -sin(3Pi/2)*xn = 1*xn
            Im += -ImPhase4 * xn;
            break;
    }

    i++;
    if (i >= N) {
        i = 0;
        Re_buff = Re;
        Im_buff = Im;
        DFT_done = 1;
        Re = 0;
        Im = 0;
    }
}

/*
 * DFT_Calc() - Calculate magnitude and phase from accumulated DFT
 * Call from main loop when DFT_done == 1
 */
void DFT_Calc(void) {
    int32_t re = Re_buff;
    int32_t im = Im_buff;

    /* Scale down to prevent overflow when squaring */
    int32_t re_scaled = re / 16;
    int32_t im_scaled = im / 16;

    /* Magnitude = sqrt(Re^2 + Im^2) */
    uint32_t mag_squared = (uint32_t)(re_scaled * re_scaled + im_scaled * im_scaled);
    magnitude = isqrt(mag_squared);

    /* Phase = atan2(Im, Re) in degrees */
    phase_deg = fast_atan2_deg(im, re);

    /* IIR Low-pass filter for display smoothing */
    magnitude_filt = (uint16_t)((IIR_ALPHA * (uint32_t)magnitude +
                      (256 - IIR_ALPHA) * (uint32_t)magnitude_filt) / 256);
    phase_filt = (int16_t)((IIR_ALPHA * (int32_t)phase_deg +
                  (256 - IIR_ALPHA) * (int32_t)phase_filt) / 256);
}
