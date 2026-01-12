/*
 * DFT Capture Mode - MATLAB Verification
 * Captures raw ADC samples and outputs via UART for verification
 */

#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>

#define N_SAMPLES 64

// Capture states
#define CAPTURE_IDLE    0
#define CAPTURE_WAITING 1
#define CAPTURE_ACTIVE  2
#define CAPTURE_DONE    3

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

#endif
