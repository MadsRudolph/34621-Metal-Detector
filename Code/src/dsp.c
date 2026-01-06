/**
 * dsp.c
 * DSP implementation: Single-bin DFT and IIR filtering
 *
 * DFT Algorithm:
 * For 2kHz signal sampled at 8kHz (4 samples per cycle):
 * Sample indices: 0, 1, 2, 3 correspond to phases 0, 90, 180, 270 degrees
 *
 * Real part:  sum of samples[i] * cos(i * 90)
 *           = samples[0]*1 + samples[1]*0 + samples[2]*(-1) + samples[3]*0
 *           = samples[0] - samples[2]
 *
 * Imag part:  sum of samples[i] * sin(i * 90)
 *           = samples[0]*0 + samples[1]*1 + samples[2]*0 + samples[3]*(-1)
 *           = samples[1] - samples[3]
 *
 * For 64 samples (16 complete cycles):
 *   real = sum(samples[4k]) - sum(samples[4k+2]) for k=0..15
 *   imag = sum(samples[4k+1]) - sum(samples[4k+3]) for k=0..15
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#include "dsp.h"
#include "config.h"
#include <math.h>

/* Filtered results storage */
static dsp_result_t filtered_result = {0.0f, 0.0f};

void dsp_init(void)
{
    filtered_result.magnitude = 0.0f;
    filtered_result.phase_deg = 0.0f;
}

void dft_calculate(const volatile uint16_t *samples, uint8_t count, dsp_result_t *result)
{
    int32_t real_sum = 0;
    int32_t imag_sum = 0;

    /*
     * Optimized single-bin DFT for 4x oversampling
     * Process 4 samples at a time (one complete cycle of reference)
     */
    for (uint8_t i = 0; i < count; i += 4) {
        /* Real: cos = {1, 0, -1, 0} */
        real_sum += (int32_t)samples[i];      /* cos(0) = 1 */
        /* samples[i+1] * cos(90) = 0 */
        real_sum -= (int32_t)samples[i + 2];  /* cos(180) = -1 */
        /* samples[i+3] * cos(270) = 0 */

        /* Imag: sin = {0, 1, 0, -1} */
        /* samples[i] * sin(0) = 0 */
        imag_sum += (int32_t)samples[i + 1];  /* sin(90) = 1 */
        /* samples[i+2] * sin(180) = 0 */
        imag_sum -= (int32_t)samples[i + 3];  /* sin(270) = -1 */
    }

    /* Convert to float for magnitude/phase calculation */
    float real_f = (float)real_sum;
    float imag_f = (float)imag_sum;

    /*
     * Magnitude calculation using actual floating-point math
     * This represents realistic CPU load for power measurement
     */
    float mag_raw = sqrtf(real_f * real_f + imag_f * imag_f);

    /*
     * Normalize magnitude to percentage
     * Max possible: count * 1023 (12-bit ADC) if all samples at peak
     * For our 64 samples: max raw ~= 64 * 1023 / 4 ~= 65520
     * Scale to 0-100%
     */
    float max_magnitude = (float)count * 1023.0f / 4.0f;
    result->magnitude = (mag_raw / max_magnitude) * 100.0f;

    /* Clamp to valid range */
    if (result->magnitude > 100.0f) {
        result->magnitude = 100.0f;
    }

    /*
     * Phase calculation using atan2f
     * Result in radians, convert to degrees
     * Use -imag for standard phase convention (positive = leading)
     */
    float phase_rad = atan2f(-imag_f, real_f);
    result->phase_deg = phase_rad * (180.0f / (float)M_PI);

    /* Normalize to 0-360 range */
    if (result->phase_deg < 0.0f) {
        result->phase_deg += 360.0f;
    }
}

float iir_filter(float new_value, float old_filtered, float alpha)
{
    return alpha * new_value + (1.0f - alpha) * old_filtered;
}

const dsp_result_t* dsp_process(const volatile uint16_t *samples, uint8_t count)
{
    dsp_result_t raw_result;

    /* Calculate raw DFT */
    dft_calculate(samples, count, &raw_result);

    /* Apply IIR filter to both magnitude and phase */
    filtered_result.magnitude = iir_filter(
        raw_result.magnitude,
        filtered_result.magnitude,
        IIR_ALPHA
    );

    /*
     * Phase filtering needs special handling for wrap-around
     * For simplicity, we filter directly (works if phase doesn't wrap often)
     */
    filtered_result.phase_deg = iir_filter(
        raw_result.phase_deg,
        filtered_result.phase_deg,
        IIR_ALPHA
    );

    return &filtered_result;
}

const dsp_result_t* dsp_get_result(void)
{
    return &filtered_result;
}
