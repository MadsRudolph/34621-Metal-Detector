# VLF Metal Detector - Assembly Guide

## Arduino Nano Breadboard Wiring

This guide shows how to wire up the Arduino Nano for the DTU 34621 Metal Detector project.

---

## Component List

| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Nano | 1 | ATmega328P, 16MHz |
| Breadboard | 1 | Full-size recommended |
| SSD1306 OLED Display | 1 | 128x64, I2C interface |
| Push buttons | 2 | Momentary, normally open |
| Piezo buzzer | 1 | 5V passive buzzer |
| TX Coil driver | 1 | H-bridge or transistor circuit |
| RX Coil amplifier | 1 | Op-amp circuit for coil signal |
| Jumper wires | ~20 | Various colors recommended |

---

## Pin Connections

### Arduino Nano Pinout Summary

```
                     +-----+
                     | USB |
                     +-----+
            +--------+-----+--------+
        TX1 | [ ]  D1|     |VIN [ ] | VIN
        RX0 | [ ]  D0|     |GND [ ] | GND
            | [ ] RST|     |RST [ ] |
            | [ ] GND|     |5V  [ ] | 5V
 INT0   [X] | [X]  D2|     |A7  [ ] |
 OC2B   [X] | [X]  D3|     |A6  [ ] |
        [X] | [X]  D4|     |A5  [X] | SCL
            | [ ]  D5|     |A4  [X] | SDA
            | [ ]  D6|     |A3  [ ] |
            | [ ]  D7|     |A2  [ ] |
 DEBUG  [X] | [X]  D8|     |A1  [ ] |
 TX-OUT [X] | [X]  D9|     |A0  [X] | RX-IN
            | [ ] D10|     |AREF[ ] |
 MOSI       | [ ] D11|     |3V3 [ ] |
 MISO       | [ ] D12|     |D13 [ ] | SCK
            +-------+-------+-------+

[X] = Used in this project
```

---

## Wiring Table

| Function | Arduino Pin | Port | Direction | Connection |
|----------|-------------|------|-----------|------------|
| **TX Signal** | D9 | PB1 | Output | TX coil driver input |
| **RX Input** | A0 | PC0 | Input | RX amplifier output |
| **Start/Stop Button** | D2 | PD2 | Input | Button to GND |
| **Calibrate Button** | D4 | PD4 | Input | Button to GND |
| **Buzzer** | D3 | PD3 | Output | Piezo buzzer + |
| **Debug Pin** | D8 | PB0 | Output | Oscilloscope probe |
| **OLED SDA** | A4 | PC4 | I2C | Display SDA |
| **OLED SCL** | A5 | PC5 | I2C | Display SCL |
| **5V** | 5V | - | Power | Display VCC, components |
| **GND** | GND | - | Power | Common ground |

---

## Wiring Diagram (ASCII)

```
                                    +------------------+
                                    |   SSD1306 OLED   |
                                    |                  |
                                    | VCC  GND  SCL SDA|
                                    +--+---+----+---+--+
                                       |   |    |   |
                                       |   |    |   |
    +----------------------------------+---+----+---+-----------+
    |                                  |   |    |   |           |
    |  ARDUINO NANO                    |   |    |   |           |
    |  +-----------+                   |   |    |   |           |
    |  |           |                   |   |    |   |           |
    |  |       5V  +-------------------+   |    |   |           |
    |  |      GND  +---[COMMON GND]--------+    |   |           |
    |  |       A5  +------------------------+   |   |           |
    |  |       A4  +----------------------------+   |           |
    |  |       A0  +---[RX AMPLIFIER OUTPUT]--------+           |
    |  |           |                                            |
    |  |       D9  +---[TX COIL DRIVER INPUT]                   |
    |  |       D8  +---[OSCILLOSCOPE PROBE (optional)]          |
    |  |           |                                            |
    |  |       D4  +---+                                        |
    |  |           |   |  +-------+                             |
    |  |           |   +--+ BTN 2 +--[GND]   (Calibrate)        |
    |  |           |      +-------+                             |
    |  |       D3  +---[BUZZER+]---[BUZZER-]---[GND]            |
    |  |           |                                            |
    |  |       D2  +---+                                        |
    |  |           |   |  +-------+                             |
    |  +-----------+   +--+ BTN 1 +--[GND]   (Start/Stop)       |
    |                     +-------+                             |
    +-----------------------------------------------------------+
```

---

## Step-by-Step Wiring Instructions

### Step 1: Place Arduino Nano on Breadboard

1. Insert the Arduino Nano straddling the center gap of the breadboard
2. Ensure all pins are firmly inserted
3. Leave space on both sides for component connections

### Step 2: Power Rails

1. Connect Arduino **5V** to breadboard **+** rail (red)
2. Connect Arduino **GND** to breadboard **-** rail (blue)
3. If using both sides, bridge the power rails

### Step 3: OLED Display (I2C)

| OLED Pin | Connect To |
|----------|------------|
| VCC | 5V rail (+) |
| GND | GND rail (-) |
| SCL | Arduino A5 |
| SDA | Arduino A4 |

### Step 4: Buttons (Active Low with Internal Pull-up)

**Start/Stop Button (D2):**
```
Arduino D2 ----+---- BTN ----+---- GND
               |             |
          (internal pull-up enabled in code)
```

**Calibrate Button (D4):**
```
Arduino D4 ----+---- BTN ----+---- GND
               |             |
          (internal pull-up enabled in code)
```

> **Note:** No external resistors needed - code enables internal pull-ups.
> Buttons connect directly between pin and GND.

### Step 5: Buzzer

| Buzzer | Connect To |
|--------|------------|
| + (positive) | Arduino D3 |
| - (negative) | GND rail |

> **Note:** For louder output, use a transistor driver circuit.

### Step 6: TX Coil Driver

Connect Arduino **D9** to your TX coil driver circuit input.

The signal characteristics:
- Frequency: 2 kHz square wave
- Voltage: 0-5V logic level
- Current: ~20mA max (use driver circuit for coil)

### Step 7: RX Coil Amplifier

Connect your RX amplifier output to Arduino **A0**.

Signal requirements:
- Voltage range: 0-5V (centered at 2.5V recommended)
- Frequency: 2 kHz (same as TX)
- Bandwidth: Amplifier should pass 2 kHz signal

### Step 8: Debug Pin (Optional)

Connect oscilloscope probe to **D8** for timing verification.

Expected signal:
- Toggle rate: 4 kHz (toggled every 8 kHz interrupt)
- Used to verify ISR timing with oscilloscope

---

## Verification Checklist

After wiring, verify each connection:

- [ ] **Power:** 5V and GND rails connected
- [ ] **OLED:** Display shows splash screen on power-up
- [ ] **Start Button:** Press D2 button - buzzer beeps, state changes
- [ ] **Calibrate Button:** Press D4 button - calibration runs
- [ ] **Debug Mode:** Hold both buttons - enters debug screen
- [ ] **TX Signal:** Oscilloscope on D9 shows 2 kHz square wave
- [ ] **Debug Pin:** Oscilloscope on D8 shows 4 kHz toggle
- [ ] **RX Input:** A0 receives signal from RX amplifier

---

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| No display | I2C wiring wrong | Check A4/A5 connections, verify I2C address |
| Buttons don't work | Wrong polarity | Buttons should connect pin to GND |
| No TX signal | Timer not running | Check sei() is called, verify D9 connection |
| Buzzer silent | Wrong pin or polarity | Verify D3 connection, check buzzer polarity |
| ADC reads 0 or 1023 | No signal or wrong range | Verify RX amp outputs 0-5V range |

---

## Safety Notes

1. **Do not exceed 5V** on any input pin
2. **Do not draw more than 20mA** from any single pin
3. **Use driver circuits** for coils - do not drive directly from pins
4. **Disconnect power** before making wiring changes

---

## Reference

- ATmega328P Datasheet: Rev. 7810D-AVR-01/15
- Arduino Nano Pinout: https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf

