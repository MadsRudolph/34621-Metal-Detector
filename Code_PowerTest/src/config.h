/**
 * config.h
 * Configuration for VLF Metal Detector Power Test
 * Target: ATmega2560 @ 16MHz (Arduino Mega 2560)
 *
 * Pin Assignments (Arduino Mega 2560 physical pinout):
 * - TX Output:  PB5 (OC1A) = Arduino Pin 11
 * - ADC Input:  ADC0       = Arduino Pin A0
 * - I2C SDA:    PD1        = Arduino Pin 20
 * - I2C SCL:    PD0        = Arduino Pin 21
 * - UART TX:    PE1        = Arduino Pin 1
 * - UART RX:    PE0        = Arduino Pin 0
 *
 * Integration: DTU 34621 Metal Detector Project
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * CPU Configuration
 * ============================================================================ */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/* ============================================================================
 * TX Signal Configuration (Timer1, 2kHz square wave)
 * ============================================================================ */
#define TX_FREQUENCY_HZ     2000
#define TX_DDR              DDRB
#define TX_PORT             PORTB
#define TX_PIN              PB5         /* OC1A = Arduino Pin 11 */

/*
 * Timer1 CTC mode with toggle on compare match:
 * f_out = F_CPU / (2 * prescaler * (1 + OCR1A))
 * OCR1A = F_CPU / (2 * prescaler * f_out) - 1
 * OCR1A = 16MHz / (2 * 1 * 2000) - 1 = 3999
 */
#define TX_OCR1A_VALUE      3999
#define TX_PRESCALER        1           /* CS10 only = /1 */

/* ============================================================================
 * Sampling Configuration (Timer3, 8kHz ISR)
 * ============================================================================ */
#define SAMPLE_FREQUENCY_HZ 8000        /* 4x oversampling of 2kHz TX */
#define SAMPLE_BUFFER_SIZE  64          /* Samples per DFT window */

/*
 * Timer3 CTC mode:
 * f_isr = F_CPU / (prescaler * (1 + OCR3A))
 * OCR3A = F_CPU / (prescaler * f_isr) - 1
 * OCR3A = 16MHz / (1 * 8000) - 1 = 1999
 */
#define SAMPLE_OCR3A_VALUE  1999
#define SAMPLE_PRESCALER    1           /* CS30 only = /1 */

/* ============================================================================
 * ADC Configuration (Onboard ATmega2560 ADC)
 * ============================================================================ */
#define ADC_CHANNEL         0           /* ADC0 = Arduino Pin A0 */
#define ADC_REFERENCE       0x40        /* AVCC with external cap at AREF */

/*
 * ADC Prescaler for 10-bit accuracy (50-200kHz ADC clock):
 * F_ADC = F_CPU / prescaler
 * 16MHz / 128 = 125kHz (within 50-200kHz range)
 * Prescaler 128 = ADPS2:ADPS1:ADPS0 = 111
 */
#define ADC_PRESCALER_BITS  ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))

/* ============================================================================
 * I2C/TWI Configuration (SSD1306 OLED)
 * ============================================================================ */
#define I2C_FREQUENCY_HZ    400000      /* 400kHz Fast Mode */
#define OLED_I2C_ADDR       0x78        /* SSD1306 write address (0x3C << 1) */

/*
 * TWI Bit Rate:
 * f_scl = F_CPU / (16 + 2 * TWBR * prescaler)
 * TWBR = (F_CPU / f_scl - 16) / (2 * prescaler)
 * TWBR = (16MHz / 400kHz - 16) / 2 = 12
 */
#define TWI_BITRATE         12

/* OLED Display dimensions */
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_PAGES          (OLED_HEIGHT / 8)

/* ============================================================================
 * UART Configuration
 * ============================================================================ */
#define UART_BAUDRATE       9600

/*
 * UBRR = F_CPU / (16 * baud) - 1
 * UBRR = 16MHz / (16 * 9600) - 1 = 103.166... ~= 103
 */
#define UART_UBRR_VALUE     103

/* ============================================================================
 * DSP Configuration
 * ============================================================================ */
#define IIR_ALPHA           0.1f        /* First-order IIR smoothing factor */
#define DFT_OVERSAMPLING    4           /* Samples per TX cycle */

/* ============================================================================
 * Display Update Configuration
 * ============================================================================ */
#define OLED_UPDATE_MS      83          /* ~12 fps */
#define SERIAL_UPDATE_MS    1000        /* Status print interval */

/* ============================================================================
 * Timing Macros
 * ============================================================================ */
#define MS_TO_TICKS(ms)     ((uint32_t)(ms))

#endif /* CONFIG_H */
