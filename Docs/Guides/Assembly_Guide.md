# VLF Metal Detector - Assembly Guide

## Arduino Nano Breadboard Wiring

This guide shows how to wire up the Arduino Nano for the DTU 34621 Metal Detector project.

**Current State:** Minimal test configuration for core TX/RX/DSP testing.

---

## Component List (Minimal Test Version)

| Component | Quantity | Notes |
|-----------|----------|-------|
| Arduino Nano | 1 | ATmega328P, 16MHz |
| Breadboard | 1 | Full-size recommended |
| SSD1306 OLED Display | 1 | 128x64, I2C interface |
| Push button | 1 | Debug screen toggle (D4) |
| TX Coil driver | 1 | H-bridge or transistor circuit |
| RX Coil amplifier | 1 | Op-amp circuit for coil signal |
| Jumper wires | ~10 | Various colors recommended |

### Components for Later (Not Used in Minimal Version)

| Component | Quantity | Notes |
|-----------|----------|-------|
| Push button | 1 | Start/Stop (D2) |
| Piezo buzzer | 1 | For audio feedback |

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
            | [ ]  D2|     |A7  [ ] |
            | [ ]  D3|     |A6  [ ] |
 DEBUG  [X] | [X]  D4|     |A5  [X] | SCL
            | [ ]  D5|     |A4  [X] | SDA
            | [ ]  D6|     |A3  [ ] |
            | [ ]  D7|     |A2  [ ] |
            | [ ]  D8|     |A1  [ ] |
 TX-OUT [X] | [X]  D9|     |A0  [X] | RX-IN
            | [ ] D10|     |AREF[ ] |
 MOSI       | [ ] D11|     |3V3 [ ] |
 MISO       | [ ] D12|     |D13 [ ] | SCK
            +-------+-------+-------+

[X] = Used in minimal test version
```

---

## Wiring Table (Minimal Test Version)

| Function | Arduino Pin | Port | Direction | Connection |
|----------|-------------|------|-----------|------------|
| **TX Signal** | D9 | PB1 | Output | TX coil driver input |
| **RX Input** | A0 | PC0 | Input | RX amplifier output |
| **Debug Button** | D4 | PD4 | Input | Button to GND |
| **OLED SDA** | A4 | PC4 | I2C | Display SDA |
| **OLED SCL** | A5 | PC5 | I2C | Display SCL |
| **5V** | 5V | - | Power | Display VCC, components |
| **GND** | GND | - | Power | Common ground |

### Pins Reserved for Future Use

| Function | Arduino Pin | Port | Notes |
|----------|-------------|------|-------|
| Start/Stop Button | D2 | PD2 | To be added |
| Buzzer | D3 | PD3 | To be added |
| Debug Pin | D8 | PB0 | Optional oscilloscope |

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
    |  |           |                                            |
    |  |       D4  +---+                                        |
    |  |           |   |  +-------+                             |
    |  +-----------+   +--+ BTN   +--[GND]   (Debug toggle)     |
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

### Step 4: TX Coil Driver

Connect Arduino **D9** to your TX coil driver circuit input.

The signal characteristics:
- Frequency: 2 kHz square wave
- Voltage: 0-5V logic level
- Current: ~20mA max (use driver circuit for coil)

### Step 5: RX Coil Amplifier

Connect your RX amplifier output to Arduino **A0**.

Signal requirements:
- Voltage range: 0-5V (centered at 2.5V recommended)
- Frequency: 2 kHz (same as TX)
- Bandwidth: Amplifier should pass 2 kHz signal

### Step 6: Debug Button (D4)

Connect a push button between **D4** and **GND**.

```
Arduino D4 ----+---- BTN ----+---- GND
               |             |
          (internal pull-up enabled in code)
```

> **Note:** No external resistor needed - code enables internal pull-up.
> Press to toggle between DFT and Debug screens.

---

## Verification Checklist (Minimal Test Version)

After wiring, verify each connection:

- [ ] **Power:** 5V and GND rails connected
- [ ] **OLED:** Display shows "Starting..." then DFT screen
- [ ] **Debug Button:** Press D4 button - screen toggles to DEBUG
- [ ] **TX Signal:** Oscilloscope on D9 shows 2 kHz square wave
- [ ] **RX Input:** A0 receives signal from RX amplifier
- [ ] **Display Values:** Re, Im, Mag, Phase values update on screen

---

## Expected Display Output

Press the **D4 button** to toggle between two screens:

**Screen 1: DFT Results**
```
=== DFT ===

Re:   <value>
Im:   <value>

Mag:  <value>
Phase: <value> deg
```

**Screen 2: Debug Info**
```
=== DEBUG ===

ADC:  <raw value>
Min:  <minimum>
Max:  <maximum>
TX:   HIGH/LOW
DFT#: <count>
Vpp:  <swing>
```

| Value | Description |
|-------|-------------|
| **Re/Im** | DFT components (can be negative) |
| **Mag** | Magnitude = sqrt(Re² + Im²) / 16 |
| **Phase** | Phase angle in degrees |
| **ADC** | Current raw ADC value (0-1023) |
| **Min/Max** | ADC range seen since power-on |
| **TX** | Current TX pin state |
| **DFT#** | Completed DFT windows (should increment) |
| **Vpp** | Peak-to-peak ADC swing |

---

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| No display | I2C wiring wrong | Check A4/A5 connections, verify I2C address |
| Display shows garbage | Wrong I2C address | Try 0x7A instead of 0x78 |
| No TX signal | Timer not running | Check sei() is called, verify D9 connection |
| Re/Im always 0 | No RX signal | Verify RX amp outputs 0-5V range on A0 |
| Re/Im always same sign | DC offset wrong | ADC should be centered at ~512 |
| Phase stuck at 0 or 90 | No signal variation | Check coil connections |

---

## Safety Notes

1. **Do not exceed 5V** on any input pin
2. **Do not draw more than 20mA** from any single pin
3. **Use driver circuits** for coils - do not drive directly from pins
4. **Disconnect power** before making wiring changes

---

## Future Additions

When ready to add UI features, wire:

| Component | Pin | Notes |
|-----------|-----|-------|
| Start/Stop Button | D2 to GND | Internal pull-up in code |
| Buzzer (+) | D3 | Negative to GND |
| Debug Pin | D8 | For oscilloscope timing |

---

## Reference

- ATmega328P Datasheet: Rev. 7810D-AVR-01/15
- Arduino Nano Pinout: https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf
