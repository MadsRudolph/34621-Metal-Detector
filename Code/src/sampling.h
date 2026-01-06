/**
 * sampling.h
 * 8kHz Sampling Timer with double-buffered 64-sample arrays
 */

#ifndef SAMPLING_H
#define SAMPLING_H

#include <stdint.h>
#include "config.h"

void adc_init(void);
void sampling_timer_init(void);
void sampling_start(void);
void sampling_stop(void);
uint8_t sampling_buffer_ready(void);
volatile uint16_t* sampling_get_buffer(void);

#endif
