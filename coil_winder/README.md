# Coil Winder Controller - Embedded C

Pure embedded C firmware for the VLF Metal Detector coil winder.

No Arduino libraries, no C++ - just AVR-libc and direct register access.

## Features

- Turn counting via external interrupt (INT0)
- Servo PWM via Timer1 hardware
- I2C SSD1306 OLED driver (TWI registers)
- Millisecond timing via Timer0
- 5x7 bitmap font rendering
- Three coil presets (TX/RX/BUCK)

## Hardware

| Pin | Function | Component |
|-----|----------|-----------|
| PD2 (D2) | INT0 | ITR8307 encoder |
| PB1 (D9) | OC1A PWM | Servo motor |
| PD3 (D3) | Input | Reset button |
| PD4 (D4) | Input | Preset button |
| PD5 (D5) | Input | Start button |
| PB2 (D10) | Output | Buzzer |
| PC4 (A4) | SDA | OLED I2C |
| PC5 (A5) | SCL | OLED I2C |

## Building with PlatformIO

```bash
cd coil_winder

# Build
pio run

# Upload
pio run -t upload

# Clean
pio run -t clean
```

## Memory Usage

```
Program:  ~4KB Flash (of 32KB)
Data:     ~1KB RAM (of 2KB)
```

## Code Structure

```
src/main.c
├── Timer0 ISR      - Millisecond counter
├── INT0 ISR        - Turn counter
├── TWI/I2C         - Hardware I2C for OLED
├── SSD1306 Driver  - Display commands and buffer
├── Font 5x7        - Basic ASCII rendering
├── Servo PWM       - Timer1 hardware PWM
├── Button Handler  - Debounced input reading
├── State Machine   - IDLE/WINDING/PAUSED/COMPLETE
└── Main Loop       - Poll buttons, process turns, update display
```

## Coil Presets

| Preset | Turns | Layers | Width | Wire |
|--------|-------|--------|-------|------|
| TX | 68 | 2 | 18mm | 0.52mm |
| RX | 170 | 4 | 14mm | 0.32mm |
| BUCK | 35 | 1 | 18mm | 0.52mm |

## Calibration

Edit these defines in `src/main.c`:

```c
#define SERVO_CENTER    90   /* Center position (degrees) */
#define SERVO_RANGE     25   /* Range each side (degrees) */
#define SERVO_MIN_US  1000   /* Minimum pulse width (us) */
#define SERVO_MAX_US  2000   /* Maximum pulse width (us) */
```

## Wiring

```
ITR8307:
  Pin 1 (Anode)    -> 330R -> 5V
  Pin 2 (Cathode)  -> GND
  Pin 3 (Collector)-> D2 + 10K pull-up to 5V
  Pin 4 (Emitter)  -> GND

Servo:
  Brown  -> GND
  Red    -> 5V
  Orange -> D9

Buttons (active low):
  D3 -> Reset button -> GND
  D4 -> Preset button -> GND
  D5 -> Start button -> GND

OLED:
  VCC -> 5V
  GND -> GND
  SDA -> A4
  SCL -> A5

Buzzer:
  + -> D10
  - -> GND
```

## Differences from C++ Version

| Feature | C++ Version | Embedded C Version |
|---------|-------------|-------------------|
| Framework | Arduino | AVR-libc only |
| OLED Driver | Adafruit library | Custom minimal driver |
| Servo | Arduino Servo lib | Timer1 hardware PWM |
| I2C | Wire library | Direct TWI registers |
| Size | ~12KB | ~4KB |
| Dependencies | 3 libraries | None |

## License

Part of DTU 34621 Metal Detector Project
