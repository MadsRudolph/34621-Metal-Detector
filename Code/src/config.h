/*
 * config.h
 * Pin definitions and constants
 * Supports both ATmega2560 (Arduino Mega) and ATmega328P (Arduino Nano)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>

/*
 * Board Detection
 * __AVR_ATmega2560__ = Arduino Mega 2560
 * __AVR_ATmega328P__ = Arduino Nano / Uno
 */

#if defined(__AVR_ATmega2560__)
/* ============================================================
 * ARDUINO MEGA 2560 (ATmega2560) Pin Configuration
 * ============================================================
 * Pins:
 *   11 (PB5) - PWM to TX coil (Timer1 OC1A)
 *   10 (PB4) - MCP3208 CS
 *   51 (PB2) - MOSI
 *   50 (PB3) - MISO
 *   52 (PB1) - SCK
 *   8  (PH5) - Buzzer (Timer4 OC4C)
 *   2  (PE4) - Start/Stop button
 *   3  (PE5) - Calibrate button
 *   13 (PB7) - Debug pin
 *   20 (PD1) - I2C SDA
 *   21 (PD0) - I2C SCL
 */

/* TX PWM - Timer1 OC1A */
#define TX_PWM_DDR      DDRB
#define TX_PWM_PORT     PORTB
#define TX_PWM_BIT      PB5     /* Pin 11 */

/* Buzzer - Timer4 OC4C */
#define BUZZER_DDR      DDRH
#define BUZZER_PORT     PORTH
#define BUZZER_BIT      PH5     /* Pin 8 */
#define BUZZER_USE_TIMER4   1

/* Buttons - Port E */
#define BTN_DDR         DDRE
#define BTN_PORT        PORTE
#define BTN_PIN         PINE
#define BTN_START_BIT   PE4     /* Pin 2 */
#define BTN_CALIB_BIT   PE5     /* Pin 3 */

/* Debug Pin */
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_BIT   PB7     /* Pin 13 */

/* I2C Startup Pin (optional) */
#define I2C_STARTUP_DDR     DDRA
#define I2C_STARTUP_PORT    PORTA
#define I2C_STARTUP_BIT     PA0
#define I2C_HAS_STARTUP_PIN 1

#elif defined(__AVR_ATmega328P__)
/* ============================================================
 * ARDUINO NANO (ATmega328P) Pin Configuration
 * ============================================================
 * Pins:
 *   9  (PB1) - PWM to TX coil (Timer1 OC1A)
 *   10 (PB2) - MCP3208 CS (SPI SS)
 *   11 (PB3) - MOSI
 *   12 (PB4) - MISO
 *   13 (PB5) - SCK
 *   3  (PD3) - Buzzer (Timer2 OC2B)
 *   2  (PD2) - Start/Stop button
 *   4  (PD4) - Calibrate button
 *   8  (PB0) - Debug pin
 *   A4 (PC4) - I2C SDA
 *   A5 (PC5) - I2C SCL
 */

/* TX PWM - Timer1 OC1A */
#define TX_PWM_DDR      DDRB
#define TX_PWM_PORT     PORTB
#define TX_PWM_BIT      PB1     /* Pin 9 */

/* Buzzer - Timer2 OC2B */
#define BUZZER_DDR      DDRD
#define BUZZER_PORT     PORTD
#define BUZZER_BIT      PD3     /* Pin 3 */
#define BUZZER_USE_TIMER2   1

/* Buttons - Port D */
#define BTN_DDR         DDRD
#define BTN_PORT        PORTD
#define BTN_PIN         PIND
#define BTN_START_BIT   PD2     /* Pin 2 */
#define BTN_CALIB_BIT   PD4     /* Pin 4 */

/* Debug Pin */
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_BIT   PB0     /* Pin 8 */

/* No startup pin on Nano */
#define I2C_HAS_STARTUP_PIN 0

#else
#error "Unsupported MCU. Please use ATmega2560 or ATmega328P."
#endif

/* ============================================================
 * Common Constants
 * ============================================================ */

/* TX Signal */
#define TX_FREQUENCY_HZ     2000    /* 2 kHz TX frequency */
#define TX_TIMER1_TOP       3999    /* 16MHz / 4000 = 4kHz toggle = 2kHz */

/* ADC Sampling */
#define SAMPLE_RATE_HZ      8000    /* 8 kHz sampling (4x oversampling) */
#define SAMPLES_PER_CYCLE   4       /* 4 samples per TX cycle */

/* DFT */
#define DFT_WINDOW_SIZE     64      /* 64 samples per DFT window */

/* Detection */
#define PHASE_THRESHOLD     65      /* Degrees: Fe vs Non-Fe boundary */
#define PHASE_HYSTERESIS    10      /* Degrees: dead band */
#define MIN_DETECTION_MAG   20      /* Minimum magnitude for detection */

/* UI */
#define DEBOUNCE_MS         50      /* Button debounce time */

#endif /* CONFIG_H */
