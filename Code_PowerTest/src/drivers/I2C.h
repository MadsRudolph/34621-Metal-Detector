/**
 * I2C.h
 * I2C (TWI) driver for ATmega328P
 *
 * ATTRIBUTION:
 * ============
 * Original Author: osch
 * Adapted from: AVR freaks (adjusted with init function)
 *
 * Hardware Connection (Arduino Uno - ATmega328P):
 * - SDA: Pin A4 (PC4)
 * - SCL: Pin A5 (PC5)
 *
 * PORTING NOTE:
 * This file was adapted from ATmega2560 to ATmega328P.
 * The TWI registers (TWBR, TWSR, TWCR, TWDR) are identical.
 * Only the GPIO port references needed updating.
 *
 * Integration: Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech
 * (DTU 34621 Metal Detector Project)
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

/* I2C clock frequency */
#define SCL_CLK 100000UL

/* CPU frequency (must match actual clock) */
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <util/delay.h>
#include <math.h>

/* Bit rate calculation macro */
#define BITRATE(TWSR) ((F_CPU/SCL_CLK)-16)/(2*pow(4,((TWSR)&((1<<TWPS0)|(1<<TWPS1)))))

/* I2C address variables (used by driver) */
extern char write_address;
extern char read_addres;

/**
 * Initialize I2C with SCL set to 100kHz
 */
void I2C_Init(void);

/**
 * I2C start function
 *
 * write_address: Device address (write mode)
 * Returns:
 *   0 = start condition fail
 *   1 = ack received
 *   2 = nack received
 *   3 = SLA+W failed
 */
uint8_t I2C_Start(char write_address);

/**
 * I2C repeated start function
 *
 * read_address: Device address (read mode)
 * Returns: Same as I2C_Start
 */
uint8_t I2C_Repeated_Start(char read_address);

/**
 * I2C write function
 *
 * data: Byte to transmit
 * Returns:
 *   0 = ack received
 *   1 = nack received
 *   2 = transmission failure
 */
uint8_t I2C_Write(char data);

/**
 * I2C read with ACK (for multi-byte reads)
 *
 * Returns: Received byte
 */
char I2C_Read_Ack(void);

/**
 * I2C read with NACK (for last byte)
 *
 * Returns: Received byte
 */
char I2C_Read_Nack(void);

/**
 * I2C stop condition
 */
void I2C_Stop(void);

#endif /* I2C_H_ */
