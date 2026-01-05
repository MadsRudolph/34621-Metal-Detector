/*
 * main.c
 * VLF Metal Detector - Main Application
 *
 * State machine: STOPPED <-> DETECTING <-> CALIBRATING
 *
 * Main loop:
 *   1. Check buttons
 *   2. Process ADC buffer when ready (DFT -> filter -> classify)
 *   3. Update display (~12 Hz)
 *   4. Update buzzer (~10 Hz)
 */

#include "config.h"

/* TODO: Implement */

int main(void)
{
    while (1) {
    }
    return 0;
}
