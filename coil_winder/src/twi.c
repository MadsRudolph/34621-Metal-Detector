/**
 * @file twi.c
 * @brief I2C/TWI communication driver
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Implementation of TWI master mode communication.
 *          Simple blocking driver without error handling (sufficient for OLED).
 */

#include "twi.h"
#include "config.h"
#include <avr/io.h>

void twi_init(void)
{
    /*
     * Set prescaler to 1 (TWPS = 0 in TWSR)
     * TWSR bits 1:0 are TWPS1:TWPS0
     */
    TWSR = 0;

    /*
     * Set bit rate for desired frequency:
     * SCL_freq = F_CPU / (16 + 2 * TWBR * prescaler)
     * TWBR = ((F_CPU / SCL_freq) - 16) / (2 * prescaler)
     *
     * At 16MHz, 400kHz, prescaler=1:
     * TWBR = ((16000000 / 400000) - 16) / 2 = (40 - 16) / 2 = 12
     */
    TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

    /* Enable TWI peripheral */
    TWCR = (1 << TWEN);
}

void twi_start(void)
{
    /*
     * Send START condition:
     * - TWINT: Clear interrupt flag (by writing 1)
     * - TWSTA: Generate START condition
     * - TWEN: Keep TWI enabled
     */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);

    /* Wait for START to be transmitted */
    while (!(TWCR & (1 << TWINT)));
}

void twi_stop(void)
{
    /*
     * Send STOP condition:
     * - TWINT: Clear interrupt flag
     * - TWSTO: Generate STOP condition
     * - TWEN: Keep TWI enabled
     *
     * Note: TWINT is not set after STOP, so no wait needed
     */
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void twi_write(uint8_t data)
{
    /* Load data into TWI data register */
    TWDR = data;

    /*
     * Start transmission:
     * - TWINT: Clear interrupt flag (triggers transmission)
     * - TWEN: Keep TWI enabled
     */
    TWCR = (1 << TWINT) | (1 << TWEN);

    /* Wait for transmission to complete */
    while (!(TWCR & (1 << TWINT)));
}
