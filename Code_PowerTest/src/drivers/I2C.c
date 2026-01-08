/**
 * I2C.c
 * I2C (TWI) driver implementation for ATmega2560
 *
 * ATTRIBUTION:
 * ============
 * Original Author: osch
 * Adapted from: AVR freaks
 *
 * Hardware Connection (Arduino Mega 2560 - ATmega2560):
 * - SDA: Pin 20 (PD1)
 * - SCL: Pin 21 (PD0)
 *
 * Integration: Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech
 * (DTU 34621 Metal Detector Project)
 */

#include "I2C.h"
#include <avr/io.h>
#include <util/delay.h>

/* Global address variables */
char write_address;
char read_addres;

/**
 * Initialize I2C with SCL set to 100kHz
 *
 * ATmega328P TWI Registers:
 * - TWBR: TWI Bit Rate Register
 * - TWSR: TWI Status Register
 * - TWCR: TWI Control Register
 */
void I2C_Init(void)
{
    /* Startup delay pin (PA0 on ATmega2560) */
    DDRA |= (1 << DDA0);
    PORTA |= (1 << PA0);

    _delay_ms(1000);        /* Startup delay for display */

    /*
     * TWI Configuration:
     * Bit Rate: TWBR = ((F_CPU / SCL_CLK) - 16) / 2
     *         = ((16000000 / 100000) - 16) / 2
     *         = (160 - 16) / 2 = 72
     *
     * But original uses 18, which gives ~400kHz with prescaler=1
     * Keep original value for compatibility with display timing.
     */
    TWBR = 18;

    /* Clear prescaler bits (prescaler = 1) */
    TWSR &= 0xFC;

    /* Enable TWI */
    TWCR = 0x05;
}

/**
 * I2C start function
 *
 * Returns:
 *   0 = start condition fail
 *   1 = ack received
 *   2 = nack received
 *   3 = SLA+W failed
 */
uint8_t I2C_Start(char write_address)
{
    uint8_t status;

    /* Generate START condition */
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /* Check status */
    status = TWSR & 0xF8;
    if (status != 0x08) {       /* START not transmitted */
        return 0;
    }

    /* Send SLA+W (slave address + write) */
    TWDR = write_address;
    TWCR = (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /* Check status */
    status = TWSR & 0xF8;
    if (status == 0x18) {       /* SLA+W transmitted, ACK received */
        return 1;
    }
    if (status == 0x20) {       /* SLA+W transmitted, NACK received */
        return 2;
    }

    return 3;                   /* SLA+W failed */
}

/**
 * I2C repeated start function
 */
uint8_t I2C_Repeated_Start(char read_address)
{
    uint8_t status;

    /* Generate repeated START */
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /* Check status */
    status = TWSR & 0xF8;
    if (status != 0x10) {       /* Repeated START not transmitted */
        return 0;
    }

    /* Send SLA+R (slave address + read) */
    TWDR = read_address;
    TWCR = (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /* Check status */
    status = TWSR & 0xF8;
    if (status == 0x40) {       /* SLA+R transmitted, ACK received */
        return 1;
    }
    if (status == 0x20) {       /* SLA+R transmitted, NACK received */
        return 2;
    }

    return 3;                   /* SLA+R failed */
}

/**
 * I2C write function
 */
uint8_t I2C_Write(char data)
{
    uint8_t status;

    /* Load data */
    TWDR = data;
    TWCR = (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /* Check status */
    status = TWSR & 0xF8;
    if (status == 0x28) {       /* Data transmitted, ACK received */
        return 0;
    }
    if (status == 0x30) {       /* Data transmitted, NACK received */
        return 1;
    }

    return 2;                   /* Transmission failure */
}

/**
 * I2C read with ACK
 */
char I2C_Read_Ack(void)
{
    /* Enable TWI with ACK generation */
    TWCR = (1 << TWEN) | (1 << TWINT) | (1 << TWEA);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}

/**
 * I2C read with NACK (last byte)
 */
char I2C_Read_Nack(void)
{
    /* Enable TWI without ACK */
    TWCR = (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}

/**
 * I2C stop condition
 */
void I2C_Stop(void)
{
    /* Generate STOP condition */
    TWCR = (1 << TWSTO) | (1 << TWINT) | (1 << TWEN);

    /* Wait for STOP to complete */
    while (TWCR & (1 << TWSTO));
}
