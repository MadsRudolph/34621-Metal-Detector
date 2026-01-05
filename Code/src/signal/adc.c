/*
 * adc.c
 * ADC sampling + PWM generation
 *
 * - Timer0: 1ms tick for millis()
 * - Timer1: 8kHz ISR for ADC sampling + PWM toggle
 * - SPI: MCP3208 12-bit ADC
 * - Double buffer: swap when main has consumed
 */

#include "adc.h"

/* TODO: Implement */
