/*
 * DFT Capture Mode - MATLAB Verification
 * Captures raw ADC samples and outputs via UART for verification
 *
 * BRUG:
 *   1. Sæt DFT_VERIFICATION_MODE til 1 for at aktivere
 *   2. Upload firmware og kør MATLAB verify_dft.m
 *   3. Sæt DFT_VERIFICATION_MODE til 0 for normal drift
 */

#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>

/*
 * ============ VERIFICATION MODE FLAG ============
 * Sæt til 1 for at aktivere MATLAB DFT verification
 * Sæt til 0 for normal drift (sparer flash og RAM)
 */
#define DFT_VERIFICATION_MODE 1

#define N_SAMPLES 64

// Capture states (skal altid være defineret for stub funktioner)
#define CAPTURE_IDLE    0
#define CAPTURE_WAITING 1
#define CAPTURE_ACTIVE  2
#define CAPTURE_DONE    3

#if DFT_VERIFICATION_MODE

// Initialize UART
void capture_init(uint32_t baud);

// State management
uint8_t capture_get_state(void);
void capture_set_state(uint8_t state);

// Store sample during DFT accumulation
void capture_store_sample(uint8_t index, int16_t sample);

// Check for serial commands (call from main loop)
void capture_check_serial(void);

// Output results via UART
void capture_output(int32_t re, int32_t im, uint16_t mag, int16_t phase);

// Send ready signal
void capture_send_ready(void);

#else

// Stub funktioner når verification mode er deaktiveret
// Compileren optimerer disse væk da de er tomme inline funktioner
static inline void capture_init(uint32_t baud) { (void)baud; }
static inline uint8_t capture_get_state(void) { return CAPTURE_IDLE; }
static inline void capture_set_state(uint8_t state) { (void)state; }
static inline void capture_store_sample(uint8_t index, int16_t sample) { (void)index; (void)sample; }
static inline void capture_check_serial(void) { }
static inline void capture_output(int32_t re, int32_t im, uint16_t mag, int16_t phase) { (void)re; (void)im; (void)mag; (void)phase; }
static inline void capture_send_ready(void) { }

#endif

#endif
