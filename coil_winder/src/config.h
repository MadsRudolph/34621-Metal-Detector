/**
 * @file config.h
 * @brief Hardware configuration and pin definitions for Coil Winder
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Central configuration file containing all hardware pin assignments,
 *          timing constants, and peripheral settings for the coil winder controller.
 *          Designed for ATmega328P (Arduino UNO) at 16MHz.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup ClockConfig Clock Configuration
 * @brief CPU clock frequency settings
 * @{
 */

/** @brief CPU clock frequency in Hz (defined via build flags or defaulted) */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/** @} */

/**
 * @defgroup EncoderPins Encoder Pin Configuration
 * @brief ITR8307 optical sensor connections (reflective type)
 * @{
 */

/** @brief Encoder input pin (PD2 = Arduino D2 = INT0) */
#define ENCODER_PIN     PD2
/** @brief Encoder data direction register */
#define ENCODER_DDR     DDRD
/** @brief Encoder port register */
#define ENCODER_PORT    PORTD
/** @brief Encoder pin input register */
#define ENCODER_PINR    PIND

/** @} */

/**
 * @defgroup ServoPins Servo Pin Configuration
 * @brief Hobby servo motor PWM output
 * @{
 */

/** @brief Servo PWM output pin (PB1 = Arduino D9 = OC1A) */
#define SERVO_PIN       PB1
/** @brief Servo data direction register */
#define SERVO_DDR       DDRB
/** @brief Servo port register */
#define SERVO_PORT      PORTB

/** @} */

/**
 * @defgroup ButtonPins Button Pin Configuration
 * @brief User input pushbuttons (active low with internal pull-ups)
 * @{
 */

/** @brief Reset button pin (PD3 = Arduino D3) */
#define BTN_RESET_PIN   PD3
/** @brief Preset cycle button pin (PD4 = Arduino D4) */
#define BTN_PRESET_PIN  PD4
/** @brief Start/Stop button pin (PD5 = Arduino D5) */
#define BTN_START_PIN   PD5
/** @brief Button data direction register */
#define BTN_DDR         DDRD
/** @brief Button port register (for pull-ups) */
#define BTN_PORT        PORTD
/** @brief Button pin input register */
#define BTN_PINR        PIND

/** @} */

/**
 * @defgroup BuzzerPins Buzzer Pin Configuration
 * @brief Piezo buzzer output for audio feedback
 * @{
 */

/** @brief Buzzer output pin (PB2 = Arduino D10) */
#define BUZZER_PIN      PB2
/** @brief Buzzer data direction register */
#define BUZZER_DDR      DDRB
/** @brief Buzzer port register */
#define BUZZER_PORT     PORTB

/** @} */

/**
 * @defgroup I2CConfig I2C/OLED Configuration
 * @brief SSD1306 OLED display settings
 * @{
 */

/** @brief SSD1306 I2C slave address (7-bit) */
#define SSD1306_ADDR    0x3C
/** @brief TWI/I2C clock frequency in Hz (400kHz fast mode) */
#define TWI_FREQ        400000UL

/** @brief OLED display width in pixels */
#define OLED_WIDTH      128
/** @brief OLED display height in pixels */
#define OLED_HEIGHT     64
/** @brief OLED display pages (height / 8) */
#define OLED_PAGES      8

/** @} */

/**
 * @defgroup ServoConfig Servo Configuration
 * @brief Servo timing and position parameters
 * @{
 */

/** @brief Minimum servo pulse width in microseconds (0 degrees) */
#define SERVO_MIN_US    1000
/** @brief Maximum servo pulse width in microseconds (180 degrees) */
#define SERVO_MAX_US    2000
/** @brief Servo center position in degrees */
#define SERVO_CENTER    90
/** @brief Servo range in degrees each side of center */
#define SERVO_RANGE     25

/** @} */

/**
 * @defgroup TimingConfig Timing Configuration
 * @brief System timing constants
 * @{
 */

/** @brief Button debounce time in milliseconds */
#define DEBOUNCE_MS     50
/** @brief Display update interval in milliseconds (10Hz refresh) */
#define DISPLAY_UPDATE_MS 100

/** @} */

#endif /* CONFIG_H */
