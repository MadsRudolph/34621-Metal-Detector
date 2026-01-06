/**
 * dsp.h
 * DSP functions: Single-bin DFT and IIR filtering
 *
 * Optimized for 4x oversampling of 2kHz signal:
 * - cos table = {1, 0, -1, 0}
 * - sin table = {0, 1, 0, -1}
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#ifndef DSP_H
#define DSP_H

#include <stdint.h>

/**
 * DSP result structure
 */
typedef struct {
    float magnitude;    /* Magnitude as percentage (0-100%) */
    float phase_deg;    /* Phase in degrees (0-360) */
} dsp_result_t;

/**
 * Calculate single-bin DFT at 2kHz for a buffer of samples
 * Uses optimized algorithm for 4x oversampling:
 * - cos(0) = 1, cos(90) = 0, cos(180) = -1, cos(270) = 0
 * - sin(0) = 0, sin(90) = 1, sin(180) = 0, sin(270) = -1
 *
 * @param samples    Pointer to sample buffer (64 samples)
 * @param count      Number of samples (should be 64)
 * @param result     Output: magnitude and phase
 */
void dft_calculate(const volatile uint16_t *samples, uint8_t count, dsp_result_t *result);

/**
 * Apply first-order IIR lowpass filter
 * y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
 *
 * @param new_value     Current input sample
 * @param old_filtered  Previous filtered output
 * @param alpha         Smoothing factor (0 < alpha <= 1)
 * @return              Filtered output
 */
float iir_filter(float new_value, float old_filtered, float alpha);

/**
 * Initialize DSP module (resets filtered values)
 */
void dsp_init(void);

/**
 * Process a buffer and update filtered results
 * Call this when a new buffer is ready
 *
 * @param samples   Pointer to sample buffer
 * @param count     Number of samples
 * @return          Pointer to filtered results
 */
const dsp_result_t* dsp_process(const volatile uint16_t *samples, uint8_t count);

/**
 * Get current filtered results without processing
 */
const dsp_result_t* dsp_get_result(void);

#endif /* DSP_H */
