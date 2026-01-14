/**
 * @file twi.h
 * @brief I2C/TWI communication driver
 * @author DTU 34621 Metal Detector Project
 * @date 2026-01-12
 *
 * @details Provides low-level TWI (Two-Wire Interface) communication for I2C devices.
 *          Used primarily to communicate with the SSD1306 OLED display.
 *          Operates in master mode at 400kHz (Fast Mode).
 *
 *          Hardware pins (fixed on ATmega328P):
 *          - SDA: PC4 (Arduino A4)
 *          - SCL: PC5 (Arduino A5)
 */

#ifndef TWI_H
#define TWI_H

#include <stdint.h>

/**
 * @brief Initialize TWI/I2C peripheral
 *
 * @details Configures the TWI hardware for 400kHz Fast Mode operation.
 *          Bit rate calculation: TWBR = ((F_CPU / TWI_FREQ) - 16) / 2
 *          At 16MHz with 400kHz: TWBR = ((16000000 / 400000) - 16) / 2 = 12
 *
 *          Enables the TWI peripheral (TWEN bit in TWCR).
 */
void twi_init(void);

/**
 * @brief Send START condition
 *
 * @details Transmits I2C START condition to begin a transaction.
 *          Blocks until START has been transmitted (TWINT flag set).
 */
void twi_start(void);

/**
 * @brief Send STOP condition
 *
 * @details Transmits I2C STOP condition to end a transaction.
 *          Does not wait for completion (STOP is handled by hardware).
 */
void twi_stop(void);

/**
 * @brief Write byte to TWI bus
 *
 * @details Transmits a single byte (address or data) on the I2C bus.
 *          Blocks until transmission complete (TWINT flag set).
 *
 *          For addressing: send (address << 1) | R/W bit
 *          - Write: (addr << 1) | 0
 *          - Read: (addr << 1) | 1
 *
 * @param data Byte to transmit (address+R/W or data byte)
 */
void twi_write(uint8_t data);

#endif /* TWI_H */
