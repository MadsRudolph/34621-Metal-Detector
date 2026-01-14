/**
 * @file servo.h
 * @brief Servo control via Timer1 PWM
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Controls a hobby servo motor for wire guide traverse using Timer1.
 *          Generates 50Hz PWM on OC1A (PB1/D9) with 1-2ms pulse width.
 *          The servo moves the wire guide back and forth during coil winding.
 *
 * @note Servo switching can cause power supply noise affecting sensitive sensors.
 *       Add 100uF capacitor on servo power rail for filtering.
 */

#ifndef SERVO_H
#define SERVO_H

#include <stdint.h>

/**
 * @brief Initialize servo PWM on Timer1/OC1A
 *
 * @details Configures Timer1 for Fast PWM mode with ICR1 as TOP.
 *          16MHz / 8 prescaler = 2MHz, ICR1 = 39999 gives 50Hz (20ms period).
 *          Initial position is center (90 degrees / 1.5ms pulse).
 *
 *          Register setup:
 *          - TCCR1A: COM1A1 (non-inverting), WGM11 (Fast PWM mode 14)
 *          - TCCR1B: WGM13+WGM12 (mode 14), CS11 (prescaler 8)
 *          - ICR1: 39999 (20ms period)
 *          - OCR1A: 3000 (1.5ms = center)
 */
void servo_init(void);

/**
 * @brief Set servo to specific angle
 *
 * @details Maps angle (0-180) to pulse width (1-2ms).
 *          Formula: pulse = 2000 + (angle * 2000 / 180) timer counts.
 *          At 2MHz timer clock: 2000 counts = 1ms, 4000 counts = 2ms.
 *
 * @param angle Angle in degrees (0-180, clamped if out of range)
 */
void servo_set_angle(uint8_t angle);

/**
 * @brief Set servo position based on traverse position
 *
 * @details Maps linear wire guide position to servo angle.
 *          Position 0 maps to (CENTER - RANGE) degrees.
 *          Position width_mm maps to (CENTER + RANGE) degrees.
 *
 *          Default: CENTER=90, RANGE=25 gives 65-115 degree sweep.
 *
 * @param position_x100 Current traverse position in mm * 100 (for 0.01mm resolution)
 * @param width_mm Total winding width in mm (from coil preset)
 */
void servo_set_position_mm(uint16_t position_x100, uint8_t width_mm);

#endif /* SERVO_H */
