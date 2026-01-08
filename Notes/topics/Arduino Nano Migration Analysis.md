# Arduino Mega 2560 to Nano Migration Analysis

> **Status: IMPLEMENTED** - All migration changes have been applied to the codebase.
> The firmware now supports both Arduino Mega 2560 and Arduino Nano via conditional compilation.

This document analyzes the feasibility of migrating the VLF Metal Detector firmware from Arduino Mega 2560 (ATmega2560) to Arduino Nano (ATmega328P).

## Summary Table

| Category | Mega 2560 | Nano (328P) | Status |
|----------|-----------|-------------|--------|
| Flash | 256 KB | 32 KB | Should fit (~15-20KB estimated) |
| SRAM | 8 KB | 2 KB | Tight but feasible (~400 bytes used) |
| Timers | Timer0-5 | Timer0-2 | **Timer4 used - BLOCKER** |
| GPIO Ports | A-L | B,C,D only | **Ports E, H, A used - BLOCKER** |
| SPI Pins | 50-52 | 11-13 | Remap required |
| I2C Pins | 20-21 | A4-A5 | Remap required |
| Serial Ports | 0-3 | 0 only | Not used in code |

---

## Blockers (Code Will NOT Compile/Run)

### 1. Timer4 Does Not Exist on ATmega328P

**Files affected:** `src/app/ui.c:32-56, 91-145`

```c
// These registers don't exist on Nano:
TCCR4A, TCCR4B, ICR4, OCR4C, COM4C1
```

The buzzer PWM uses Timer4 (16-bit). On the Nano, you must use **Timer2** (8-bit) or repurpose **Timer0/Timer1**.

**Solution:** Rewrite buzzer code to use Timer2:
- OC2A (Pin 11, PB3) or OC2B (Pin 3, PD3)
- Timer2 is 8-bit, so frequency resolution is lower

---

### 2. Port E Does Not Exist (Buttons)

**Files affected:** `src/app/ui.c:38-39, 65-66, 71, 77` and `src/main.c:106-107, 112-113, 126`

```c
// These don't exist on ATmega328P:
DDRE, PORTE, PINE, PE4, PE5
```

Buttons on pins 2 (PE4) and 3 (PE5) use Port E registers directly.

**Solution:** Use Port D instead:
```c
// Replace:
DDRE &= ~((1 << PE4) | (1 << PE5));
PORTE |= (1 << PE4) | (1 << PE5);

// With:
DDRD &= ~((1 << PD2) | (1 << PD3));
PORTD |= (1 << PD2) | (1 << PD3);
```

---

### 3. Port H Does Not Exist (Buzzer Output)

**Files affected:** `src/app/ui.c:45`

```c
DDRH |= (1 << PH5);  // Port H doesn't exist on ATmega328P
```

**Solution:** Use a different pin with Timer2 PWM output:
- Pin 3 (PD3/OC2B) or Pin 11 (PB3/OC2A)

---

### 4. Port A Does Not Exist (I2C Startup Delay)

**Files affected:** `src/drivers/I2C.c:36-38`

```c
DDRA |= (1 << DDA0);   // Port A doesn't exist
PORTA |= (1 << PA0);
```

**Solution:** Remove this code or use a different available pin (e.g., PB0).

---

### 5. TX PWM Pin Mapping Conflict

**Files:** `config.h` mentions Pin 9 (PH6), but `tx.c` uses PB5 (Pin 11)

```c
// tx.c line 5:
DDRB |= (1 << PB5);   /* Pin 11 = OC1A on Mega */
```

On Nano:
- PB5 = Pin 13 (conflicts with LED and SPI SCK)
- Timer1 OC1A = Pin 9 (PB1)

**Solution:** Change to use PB1 (Pin 9) for TX PWM:
```c
DDRB |= (1 << PB1);   /* Pin 9 = OC1A on Nano */
```

---

## Warnings (Need Adjustment)

### 1. SPI Pin Remapping Required

| Function | Mega Pin | Mega Port | Nano Pin | Nano Port |
|----------|----------|-----------|----------|-----------|
| MOSI | 51 | PB2 | 11 | PB3 |
| MISO | 50 | PB3 | 12 | PB4 |
| SCK | 52 | PB1 | 13 | PB5 |
| CS | 10 | PB4 | 10 | PB2 |

The SPI hardware handles this automatically, but any direct port manipulation code needs updating.

---

### 2. I2C Pin Remapping Required

| Function | Mega Pin | Mega Port | Nano Pin | Nano Port |
|----------|----------|-----------|----------|-----------|
| SDA | 20 | PD1 | A4 | PC4 |
| SCL | 21 | PD0 | A5 | PC5 |

The TWI hardware handles routing automatically. The I2C driver uses standard TWI registers (TWBR, TWSR, TWCR, TWDR) which are identical on both chips.

---

### 3. ADC Auto-Trigger Source

**File:** `src/signal/rx.c:51`

```c
ADCSRB = (1 << ADTS2) | (1 << ADTS0);  // Timer1 Compare B trigger
```

The ADTS bits are the same on ATmega328P (0b101 = Timer1 Compare Match B), so this should work without changes.

---

### 4. Debug Pin Conflict

**File:** `src/app/debug.h:120-122`

```c
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_BIT   PB7  // Pin 13 on Mega
```

On Nano, PB7 doesn't exist (Port B only has PB0-PB5).

**Solution:** Use PB0 (Pin 8) instead:
```c
#define DEBUG_PIN_BIT   PB0  // Pin 8 on Nano
```

---

### 5. Memory Optimization Recommended

**Current SRAM Usage Estimate:**

| Component | Bytes | Source |
|-----------|-------|--------|
| ADC buffer (64 x uint16_t) | 128 | rx.c |
| DFT accumulators (Re, Im, buffers) | 26 | dsp.c |
| Debug variables | 41 | debug.c |
| UI state | 4 | ui.c |
| Detector state | 8 | detector.c |
| String buffer | 12 | debug.c |
| **Subtotal** | **~220** | |
| Stack + local variables | ~200-400 | |
| **Total Estimate** | **420-620 bytes** | |

With 2KB (2048 bytes) SRAM on Nano, there is adequate headroom, but consider:

1. **Remove debug system for production** - saves ~100 bytes of RAM
2. **Reduce ADC_BUFFER_SIZE** from 64 to 32 if acceptable for your DFT
3. **Use F() macro** for any string literals (if Arduino framework is used)

---

## What Works Without Changes

| Component | Reason |
|-----------|--------|
| Timer1 for TX/ADC | Same 16-bit timer on both chips |
| DFT algorithm | Pure integer math, fully portable |
| I2C TWI registers | TWBR, TWSR, TWCR, TWDR identical |
| ADC registers | ADMUX, ADCSRA, ADCSRB identical |
| No Serial1/2/3 | Code is bare-metal, no serial used |
| No external libraries | Pure AVR-libc, fully compatible |
| Detector logic | No hardware dependencies |
| Display driver | Uses standard I2C/TWI |

---

## Recommended Pin Mapping for Nano

```c
// Arduino Nano ATmega328P Pin Mapping
// config.h

#ifndef CONFIG_H
#define CONFIG_H

/* TX Signal Generation */
#define TX_PWM_DDR      DDRB
#define TX_PWM_PORT     PORTB
#define TX_PWM_BIT      PB1     // Pin 9 (OC1A)

/* SPI for MCP3208 ADC */
#define SPI_CS_DDR      DDRB
#define SPI_CS_PORT     PORTB
#define SPI_CS_BIT      PB2     // Pin 10 (SS)
// MOSI = Pin 11 (PB3), MISO = Pin 12 (PB4), SCK = Pin 13 (PB5)

/* Buzzer PWM (Timer2) */
#define BUZZER_DDR      DDRD
#define BUZZER_PORT     PORTD
#define BUZZER_BIT      PD3     // Pin 3 (OC2B)

/* Buttons */
#define BTN_DDR         DDRD
#define BTN_PORT        PORTD
#define BTN_PIN         PIND
#define BTN_START_BIT   PD2     // Pin 2
#define BTN_CALIB_BIT   PD4     // Pin 4 (PD3 used by buzzer)

/* Debug Pin */
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_BIT   PB0     // Pin 8

/* I2C - Hardware TWI */
// SDA = A4 (PC4), SCL = A5 (PC5) - handled by TWI hardware

#endif /* CONFIG_H */
```

---

## Migration Steps

### Step 1: Update platformio.ini

```ini
[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino

board_build.f_cpu = 16000000L

build_flags =
    -std=c99
    -I src
    -I src/signal
    -I src/app
    -I src/drivers
    -Wall
    -Os

build_src_filter =
    +<*.c>
    +<signal/*.c>
    +<app/*.c>
    +<drivers/*.c>

monitor_speed = 115200
upload_speed = 115200
```

### Step 2: Update tx.c

```c
void timer1_init(void) {
    DDRB |= (1 << PB1);   /* Pin 9 = OC1A on Nano */

    /* CTC mode, toggle OC1A on compare match */
    TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12) | (1 << CS10);

    OCR1A = 3999;  /* 16 MHz / 4000 = 2 kHz */
}
```

### Step 3: Rewrite ui.c for Timer2 Buzzer

```c
void ui_init(void)
{
    /* Buttons on PD2 and PD4 */
    DDRD &= ~((1 << PD2) | (1 << PD4));
    PORTD |= (1 << PD2) | (1 << PD4);

    /* Buzzer on PD3 (OC2B) - Timer2 Fast PWM */
    DDRD |= (1 << PD3);

    /* Timer2: Fast PWM, TOP = 0xFF */
    TCCR2A = (1 << WGM21) | (1 << WGM20);
    TCCR2B = (1 << CS22);  /* Prescaler 64 */

    /* Start with buzzer off */
    OCR2B = 0;
}

void buzzer_on(void)
{
    TCCR2A |= (1 << COM2B1);
    OCR2B = 127;  /* 50% duty */
}

void buzzer_off(void)
{
    TCCR2A &= ~(1 << COM2B1);
    OCR2B = 0;
}
```

### Step 4: Update Button Reading in ui.c and main.c

Replace all instances of:
- `PINE` with `PIND`
- `PE4` with `PD2`
- `PE5` with `PD4`

### Step 5: Update debug.h

```c
#define DEBUG_PIN_PORT  PORTB
#define DEBUG_PIN_DDR   DDRB
#define DEBUG_PIN_BIT   PB0     // Pin 8
```

### Step 6: Remove Port A Code from I2C.c

Delete or comment out lines 36-38:
```c
// DDRA |= (1 << DDA0);
// PORTA |= (1 << PA0);
```

### Step 7: Build and Test

```bash
pio run -e nano
pio run -e nano -t upload
pio device monitor
```

---

## Final Verdict

| Aspect | Assessment |
|--------|------------|
| **Feasibility** | Possible with significant changes |
| **Effort Level** | Medium-High (1-2 days of refactoring) |
| **Primary Work** | Timer4 to Timer2, GPIO remapping |
| **Risk Areas** | Buzzer frequency range, memory limits |
| **Recommendation** | Proceed with migration |

The core signal processing (TX, RX, DFT, detection algorithm) is well-designed using portable integer math. The main work involves adapting GPIO and timer configurations for the ATmega328P's reduced peripheral set.

---

## Implementation Status

The following changes have been implemented in the codebase:

| File | Changes Made |
|------|--------------|
| `src/config.h` | Complete rewrite with conditional compilation for both MCUs |
| `src/signal/tx.c` | Uses config.h macros for TX PWM pin |
| `src/app/ui.c` | Timer4/Timer2 buzzer support, portable button handling |
| `src/main.c` | Uses BTN_PIN macros for button reading |
| `src/app/debug.h` | Includes config.h for debug pin definitions |
| `src/app/debug.c` | Includes config.h |
| `src/drivers/I2C.c` | Conditional Port A startup code |
| `platformio.ini` | Added `[env:nano]` and `[env:nano_old]` environments |

### Build Commands

```bash
# Build for Arduino Mega 2560
pio run -e mega

# Build for Arduino Nano
pio run -e nano

# Upload to Mega
pio run -e mega -t upload

# Upload to Nano
pio run -e nano -t upload

# Upload to Nano (old bootloader)
pio run -e nano_old -t upload
```

### Pin Mapping Quick Reference

| Function | Mega Pin | Nano Pin |
|----------|----------|----------|
| TX PWM | 11 (PB5) | 9 (PB1) |
| Buzzer | 8 (PH5) | 3 (PD3) |
| Start Button | 2 (PE4) | 2 (PD2) |
| Calibrate Button | 3 (PE5) | 4 (PD4) |
| Debug Pin | 13 (PB7) | 8 (PB0) |
| SPI CS | 10 (PB4) | 10 (PB2) |
| I2C SDA | 20 (PD1) | A4 (PC4) |
| I2C SCL | 21 (PD0) | A5 (PC5) |

---

## References

- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
- [Arduino Nano Pinout](https://docs.arduino.cc/hardware/nano)
- [AVR Libc Reference](https://www.nongnu.org/avr-libc/user-manual/)
