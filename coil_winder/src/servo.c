/**
 * @file servo.c
 * @brief Servo control via Timer1 PWM
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of 50Hz servo PWM using Timer1 Fast PWM mode.
 *          Outputs on OC1A (PB1 = Arduino D9).
 */

#include "servo.h"
#include "config.h"
#include <avr/io.h>

void servo_init(void)
{
    /* Configure PB1 (OC1A) as output */
    SERVO_DDR |= (1 << SERVO_PIN);

    /*
     * Timer1 Fast PWM Mode 14 (WGM13:10 = 1110)
     * - TOP = ICR1
     * - Non-inverting output on OC1A (COM1A1:0 = 10)
     * - Prescaler 8 (CS11)
     */
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);

    /*
     * Period calculation:
     * f_timer = 16MHz / 8 = 2MHz
     * Period = (ICR1 + 1) / 2MHz = 40000 / 2MHz = 20ms = 50Hz
     */
    ICR1 = 39999;

    /*
     * Center position (1.5ms pulse):
     * 1.5ms * 2MHz = 3000 counts
     */
    OCR1A = 3000;
}

void servo_set_angle(uint8_t angle)
{
    /* Clamp angle to valid range */
    if (angle > 180) {
        angle = 180;
    }

    /*
     * Map angle to pulse width:
     * 0 degrees   = 1.0ms = 2000 counts
     * 180 degrees = 2.0ms = 4000 counts
     *
     * pulse = 2000 + (angle * 2000 / 180)
     */
    uint16_t pulse = 2000 + ((uint32_t)angle * 2000 / 180);
    OCR1A = pulse;
}

void servo_set_position_mm(uint16_t position_x100, uint8_t width_mm)
{
    /*
     * Map linear position to servo angle:
     * position = 0         -> angle = CENTER - RANGE (e.g., 65 degrees)
     * position = width_mm  -> angle = CENTER + RANGE (e.g., 115 degrees)
     *
     * angle_offset = (position / width) * RANGE * 2 - RANGE
     * angle = CENTER + angle_offset
     */
    uint16_t width_x100 = (uint16_t)width_mm * 100;

    int16_t angle_offset = ((int32_t)position_x100 * SERVO_RANGE * 2 / width_x100) - SERVO_RANGE;
    int16_t angle = SERVO_CENTER + angle_offset;

    /* Clamp to valid servo range */
    if (angle < 0) {
        angle = 0;
    }
    if (angle > 180) {
        angle = 180;
    }

    servo_set_angle((uint8_t)angle);
}
