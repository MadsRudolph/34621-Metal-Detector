/**
 * I2C.c
 * I2C (TWI) driver implementation for Arduino Nano (ATmega328P)
 *
 * ATTRIBUTION:
 * ============
 * Original Author: osch
 * Adapted from: AVR freaks
 *
 * Hardware Connection (Arduino Nano):
 *   SDA = A4 (PC4)
 *   SCL = A5 (PC5)
 *
 * ÆNDRING: Fjernet config.h og ATmega2560 kode - kun Nano support.
 *
 * Integration: Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech
 * (DTU 34621 Metal Detector Project)
 *
 * ATmega328P Datasheet Reference (Rev. 7810D–AVR–01/15):
 * ======================================================
 * TWI (I2C):
 *   - TWBR:        Section 21.9.1, Page 198
 *   - TWSR:        Section 21.9.3, Page 200
 *   - TWDR:        Section 21.9.4, Page 200
 *   - TWCR:        Section 21.9.2, Page 199
 *   - SCL freq:    Section 21.5.2, Page 180
 *   - Status MT:   Table 21-3, Page 186
 *   - Status MR:   Table 21-4, Page 189
 *   - Prescaler:   Table 21-8, Page 200
 */

#include "I2C.h"
#include <avr/io.h>
#include <util/delay.h>

/*
 * FJERNET: #include "../config.h"
 *
 * FJERNET: I2C_HAS_STARTUP_PIN og tilhørende kode.
 * Startup pin (PA0) var kun tilgængelig på ATmega2560 (Mega).
 * Arduino Nano har ikke denne pin, så koden er fjernet.
 */

/* Global address variables */
char write_address;
char read_addres;

/**
 * Initialize the TWI (I2C) hardware for Arduino Nano (ATmega328P) with SCL ≈308 kHz.
 *
 * Configures the TWI bit rate and prescaler, enables the TWI peripheral and TWI interrupt,
 * and performs a 1000 ms startup delay.
 *
 * Hardware effects:
 * - Sets TWBR = 18 and prescaler = 1 (TWSR prescaler bits cleared) producing SCL ≈ 16000000 / (16 + 2*18) ≈ 308 kHz.
 * - Enables TWI and TWI interrupt (TWEN and TWIE).
 * - Waits 1000 ms to allow display/startup circuitry to settle.
 *
 * I2C pins on Nano (ATmega328P):
 * - SDA: A4 (PC4)
 * - SCL: A5 (PC5)
 */
void I2C_Init(void)
{
    /*
     * FJERNET: Startup pin kode
     *
     * ORIGINAL KODE:
     *   #if I2C_HAS_STARTUP_PIN
     *       I2C_STARTUP_DDR |= (1 << I2C_STARTUP_BIT);
     *       I2C_STARTUP_PORT |= (1 << I2C_STARTUP_BIT);
     *   #endif
     *
     * HVORFOR FJERNET:
     * - I2C_STARTUP_PIN (PA0) findes ikke på ATmega328P
     * - Nano har ikke Port A overhovedet
     */

    _delay_ms(1000);        /* Startup delay for display */

    /*
     * TWI Configuration:
     * SCL Frequency: SCL = F_CPU / (16 + 2 * TWBR * Prescaler)
     *
     * Med TWBR = 18, Prescaler = 1:
     * SCL = 16000000 / (16 + 2 * 18 * 1) = 16000000 / 52 = ~308 kHz
     *
     * Datasheet: Section 21.5.2, Page 180 - Bit Rate Generator
     * Datasheet: Section 21.9.1, Page 198 - TWBR register
     */
    TWBR = 18;

    /*
     * Clear prescaler bits (prescaler = 1)
     * TWPS1:0 = 00 gives prescaler = 1
     * Datasheet: Section 21.9.3, Page 200 - TWSR register
     * Datasheet: Table 21-8, Page 200 - TWI Bit Rate Prescaler
     */
    TWSR &= 0xFC;

    /*
     * Enable TWI
     * TWCR = 0x05 = TWEN (bit 2) + TWIE (bit 0)
     * Datasheet: Section 21.9.2, Page 199 - TWCR register
     */
    TWCR = 0x05;
}

/**
 * Generate an I2C START condition and send the SLA+W byte to the slave.
 *
 * Sends a START (or repeated START) on the TWI bus, transmits the provided
 * SLA+W byte, and returns the resulting bus status code interpreted for
 * master-transmitter write initiation.
 *
 * @param write_address SLA+W byte to transmit (slave address with the write bit).
 * @returns `0` if START condition was not transmitted; `1` if SLA+W was transmitted and ACK received;
 *          `2` if SLA+W was transmitted and NACK received; `3` if SLA+W transmission resulted in an unexpected status. 
 */
uint8_t I2C_Start(char write_address)
{
    uint8_t status;

    /*
     * Generate START condition
     * TWSTA = 1: Generate START
     * TWEN  = 1: Enable TWI
     * TWINT = 1: Clear interrupt flag (starts operation)
     * Datasheet: Section 21.9.2, Page 199 - TWCR register
     */
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete (TWINT becomes 1) */
    while (!(TWCR & (1 << TWINT)));

    /*
     * Check status (mask prescaler bits)
     * 0x08 = START transmitted
     * Datasheet: Table 21-3, Page 186 - Status code 0x08
     */
    status = TWSR & 0xF8;
    if (status != 0x08) {
        return 0;
    }

    /*
     * Send SLA+W (slave address + write)
     * Datasheet: Section 21.9.4, Page 200 - TWDR register
     */
    TWDR = write_address;
    TWCR = (1 << TWEN) | (1 << TWINT);

    /* Wait for TWI to complete */
    while (!(TWCR & (1 << TWINT)));

    /*
     * Check status
     * 0x18 = SLA+W transmitted, ACK received
     * 0x20 = SLA+W transmitted, NACK received
     * Datasheet: Table 21-3, Page 186 - Status codes 0x18, 0x20
     */
    status = TWSR & 0xF8;
    if (status == 0x18) {
        return 1;
    }
    if (status == 0x20) {
        return 2;
    }

    return 3;
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