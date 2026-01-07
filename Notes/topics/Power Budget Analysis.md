# Power Budget Analysis & Current Measurement Guide

> **Requirement:** Metal detector must run for 100 minutes on a 9V battery (6LR61) with remaining voltage >6V after the 100 minutes.
>
> See: [[kravspecifikation.pdf|Kravspecifikation]] requirements 4 & 5

---

## Battery Specifications (Duracell MN1604)

> **Datasheet:** [[Literature/duracell_9volt.pdf|Duracell 9V Datasheet]]

|Parameter|Value|
|---|---|
|Nominal Voltage|9V|
|Operating Voltage|9.6V - 4.8V|
|Internal Impedance|1.7 @ 1kHz|
|Capacity (to 6V)|~400-500 mAh (load dependent)|

### Discharge Curves (from datasheet)

|Constant Current|Runtime to 6V|
|---|---|
|10 mA|>400 hours|
|50 mA|~10 hours (600 min)|
|100 mA|~3.5 hours (210 min)|
|150 mA|~2 hours (120 min)|

---

## Power Budget Calculation

### Requirement

- Runtime: **100 minutes minimum**
- End voltage: **>6V**
- Safety margin: **1.5x** recommended

### Maximum Allowable Current

From the discharge curves, to guarantee 100 min with margin:

$$I_{max} = 120\ \text{mA (conservative)}$$

With 1.5x safety margin for temperature and battery variation:

$$I_{target} = \frac{120\ \text{mA}}{1.5} = 80\ \text{mA}$$

> **Target total system current: 80-120 mA**

---

## Power Test Firmware

Simple prime number calculator to measure baseline current consumption with CPU load and OLED display active.

### Code Structure (`Code/src/`)

```
Code/
├── platformio.ini          # PlatformIO build configuration
└── src/
    ├── main.c              # Prime number calculator
    ├── config.h            # CPU frequency, I2C settings
    └── drivers/
        ├── I2C.c / I2C.h   # TWI/I2C 400kHz driver
        ├── ssd1306.c / ssd1306.h  # OLED display driver
        └── data.h          # Font data
```

### Build & Upload (PlatformIO)

```bash
# Build the firmware
cd Code
pio run

# Upload to Arduino Mega
pio run -t upload
```

### System Operation

- **Prime calculation** - CPU continuously calculates prime numbers (simulates processing load)
- **SSD1306 OLED** - Display updates every 500ms showing:
  - Current prime number found
  - Total count of primes
  - Square of current prime

---

## Breadboard Wiring

### Pin Connections (Arduino Mega 2560)

| Component | Arduino Pin | ATmega2560 Pin | Notes |
|-----------|-------------|----------------|-------|
| **SSD1306 OLED** ||||
| VCC | 5V | - | 3.3V also works for some modules |
| GND | GND | - | |
| SDA | Pin 20 | PD1 | I2C Data |
| SCL | Pin 21 | PD0 | I2C Clock |

### Wiring Diagram

```
Arduino Mega 2560
┌─────────────────────────────────────┐
│                                     │
│  5V ●────────────────┐              │
│ GND ●──────────┐     │              │
│                │     │              │
│  20 (SDA) ●────┼─────┼──────┐       │
│  21 (SCL) ●────┼─────┼────┐ │       │
│                │     │    │ │       │
└─────────────────────────────────────┘
                 │     │    │ │
                 │     │    │ │
            ┌────┴─────┴────┴─┴────┐
            │   SSD1306 OLED       │
            │  ┌───┬───┬───┬───┐   │
            │  │GND│VCC│SCL│SDA│   │
            │  └───┴───┴───┴───┘   │
            └──────────────────────┘
```

### OLED Module Pinout

Most SSD1306 modules have 4 pins (some have 7). Common 4-pin layout:

| Pin | Connection |
|-----|------------|
| GND | Arduino GND |
| VCC | Arduino 5V (or 3.3V) |
| SCL | Arduino Pin 21 |
| SDA | Arduino Pin 20 |

> **I2C Address:** The code uses 0x3C (7-bit) / 0x78 (8-bit write address). Most SSD1306 modules default to this. If display doesn't work, try 0x3D.

---

## Current Measurement Methods

Two methods are available depending on what you're testing:

| Method | Best For | Requirements | Limitations |
|--------|----------|--------------|-------------|
| **AD3 Power Supply** | Quick component measurements, debugging | AD3 only | 250mA max, 5V only, no discharge test |
| **1Ω Shunt + Battery** | Full system, TX coil, 100-min verification | 1Ω resistor, 9V battery | Requires resistor |

### Decision Guide
```
What are you testing?
│
├─► Individual components (MCU, OLED, DSP overhead)
│   └─► Use Method 1: AD3 Power Supply
│
├─► Full system WITH TX coil driver
│   └─► Use Method 2: 1Ω Shunt + Battery
│
└─► 100-minute runtime verification
    └─► Use Method 2: 1Ω Shunt + Battery (REQUIRED)
```

---

## Method 1: AD3 Power Supply (Quick Measurements)

The AD3 has a built-in programmable power supply that displays current draw directly — no shunt resistor needed.

### When to Use
- ✅ Measuring Arduino Mega baseline current
- ✅ Measuring OLED display current contribution
- ✅ Measuring DSP/firmware overhead
- ✅ Quick debugging during development
- ❌ NOT for TX coil testing (may exceed 250mA)
- ❌ NOT for 100-minute verification (can't simulate battery discharge)

### Equipment Needed

- [ ] Analog Discovery 3
- [ ] Arduino Mega 2560 (with power test firmware)
- [ ] SSD1306 OLED display
- [ ] Jumper wires

> **Note:** No external resistors required!

### Wiring Diagram
```
Analog Discovery 3                    Arduino Mega 2560
┌─────────────────┐                  ┌─────────────────┐
│                 │                  │                 │
│  V+ (red)     ●─┼──────────────────┼─● 5V            │
│                 │                  │                 │
│  GND (black)  ●─┼──────────────────┼─● GND           │
│                 │                  │                 │
└─────────────────┘                  └─────────────────┘
```

**Critical — DO NOT connect:**
- USB cable (disconnect after programming)
- 9V battery
- TX coil driver circuit
- Any other power source

### Why 5V Pin Instead of Vin?

| Connection | Pros | Cons |
|------------|------|------|
| **5V pin (recommended)** | Accurate reading, bypasses lossy regulator | 250mA limit |
| Vin | Higher current capacity | AD3 would need 7-9V, less accurate |

### AD3 Limits

| Parameter | Value |
|-----------|-------|
| V+ Max Voltage | 5.0V |
| V+ Max Current | **250 mA** |
| Protection | Auto-shutdown on overcurrent |

> ⚠️ **Warning:** Do NOT connect the TX coil driver — electronics only!

### WaveForms Configuration

1. Launch **WaveForms**
2. Open **Supplies** instrument (Welcome tab or Window → Supplies)
3. Configure:

| Setting | Value |
|---------|-------|
| Positive Supply (V+) | **5.00 V** |
| Negative Supply (V-) | Off (0V) |
| Master Enable | **ON** |

4. Click **gear icon** (⚙️) → Enable **"Show Current"**

### Reading Current

Current displays directly in milliamps — no conversion needed:
```
┌─────────────────────────────────────────┐
│  Supplies                               │
├─────────────────────────────────────────┤
│  Positive Supply (V+)                   │
│  ┌─────────────────────────────────┐    │
│  │  Voltage: 5.00 V                │    │
│  │  Current: 78.4 mA  ◄────────────┼────┼── Direct reading
│  └─────────────────────────────────┘    │
│                                         │
│  [====== Master Enable: ON ======]      │
└─────────────────────────────────────────┘
```

### Test Procedure (AD3 Method)

1. **Flash firmware:** `pio run -t upload`
2. **Disconnect USB** from Arduino
3. **Connect AD3:** V+ → 5V pin, GND → GND
4. **Open WaveForms → Supplies**
5. **Set V+ = 5.00V**, enable current display
6. **Enable Master** — Arduino powers on
7. **Wait 30 seconds** for stable reading
8. **Record I+ value**

### Component Measurement Log (AD3 Method)

| # | Configuration | I+ (mA) | Notes |
|---|---------------|---------|-------|
| 1 | Arduino Mega (blank sketch) | | Baseline |
| 2 | Power test firmware running | | DSP active |
| 3 | + SSD1306 OLED connected | | Display updating |
| 4 | Full electronics (no TX coil) | | Final electronics measurement |

### Calculate Component Breakdown

| Component | Current (mA) | Calculation |
|-----------|--------------|-------------|
| Arduino Mega (bare) | | Measurement #1 |
| Firmware overhead | | #2 - #1 |
| SSD1306 OLED | | #3 - #2 |
| **Total Electronics** | | #4 |
| **Budget for TX Coil** | | 120mA - #4 |

---

## Method 2: 1Ω Shunt Resistor + 9V Battery (Full System Test)

Use this method for testing the complete system including TX coil driver, and for the mandatory 100-minute runtime verification.

### When to Use
- ✅ Full system current (electronics + TX coil)
- ✅ TX coil driver testing
- ✅ **100-minute runtime verification (REQUIRED)**
- ✅ Verifying battery voltage stays >6V
- ✅ Real-world operating conditions

### Equipment Needed

- [ ] Analog Discovery 3
- [ ] **1Ω precision resistor** (1% tolerance, ≥0.5W power rating)
- [ ] 9V battery (fresh Duracell MN1604)
- [ ] Arduino Mega 2560 (with power test firmware)
- [ ] SSD1306 OLED display
- [ ] Complete TX coil driver circuit
- [ ] Jumper wires
- [ ] Multimeter (to verify battery voltage)

### Wiring Diagram
```
                      ┌─────────┐
9V Battery (+) ───────┤  1Ω     ├──────┬────────► Arduino Vin
                      └─────────┘      │
                            │          │
             Scope 1+ ──────┘          │
             (orange)                  │
                                       │
             Scope 1- ─────────────────┘
             (orange/white)

9V Battery (-) ────────────────────────────────► Arduino GND
                            │
             AD3 GND ───────┘ (black wire)
```

### Connection Summary

| Wire | From | To |
|------|------|----|
| Red | 9V Battery (+) | Resistor input |
| - | Resistor output | Arduino Vin |
| Scope 1+ (orange) | - | Resistor input (battery side) |
| Scope 1- (orange/white) | - | Resistor output (Vin side) |
| Black | 9V Battery (-) | Arduino GND |

### Current Calculation

Ohm's Law with 1Ω resistor:

$I = \frac{V_{shunt}}{R} = \frac{V_{shunt}}{1\Omega}$

**Conversion: 1 mV = 1 mA**

| Scope Reading | Current |
|---------------|---------|
| 50 mV | 50 mA |
| 80 mV | 80 mA |
| 100 mV | 100 mA |
| 120 mV | 120 mA |

> **Note:** Reading may be negative depending on probe polarity — use absolute value.

### Voltage Drop Consideration

The 1Ω resistor drops voltage:

| Current | Voltage Drop | Remaining for Arduino |
|---------|--------------|----------------------|
| 50 mA | 50 mV | 8.95V ✅ |
| 100 mA | 100 mV | 8.90V ✅ |
| 150 mA | 150 mV | 8.85V ✅ |

At 1Ω the drop is negligible (<200mV) and won't affect operation.

### WaveForms Scope Configuration

| Setting | Value |
|---------|-------|
| Channel 1 | Enabled |
| Coupling | DC |
| Range | **200 mV/div** |
| Offset | 0V |
| Time Base | **500 ms/div** |
| Trigger | Auto (free running) |

**Measurements to add:**
- **Average** — Your current in mA
- **Min / Max** — See fluctuations
- **RMS** — For varying loads

### Test Procedure (Shunt Method)

1. **Flash firmware:** `pio run -t upload`
2. **Disconnect USB** from Arduino
3. **Wire shunt circuit** as shown above
4. **Measure battery voltage** with multimeter: ______V
5. **Open WaveForms → Scope**
6. **Configure** per table above
7. **Connect battery** — system powers on
8. **Wait 30 seconds** for stable reading
9. **Record Average** measurement
10. **Convert:** mV reading = mA current

### Full System Measurement Log (Shunt Method)

| # | Configuration | Voltage (mV) | Current (mA) | Notes |
|---|---------------|--------------|--------------|-------|
| 1 | Electronics only (no TX) | | | Baseline |
| 2 | + TX driver (no coil) | | | Driver overhead |
| 3 | + TX coil connected | | | Full system |
| 4 | Full system, detecting metal | | | Peak current |

---

## 100-Minute Runtime Verification Test

> **Requirement:** Must run 100 minutes on 9V battery with final voltage >6V
>
> ⚠️ **This test REQUIRES Method 2 (1Ω Shunt + Battery)**

### Pre-Test Checklist

- [ ] Fresh 9V Duracell MN1604 battery
- [ ] Battery voltage measured: ______V (should be >9.0V)
- [ ] 1Ω shunt resistor installed
- [ ] Complete system assembled (electronics + TX coil)
- [ ] AD3 scope configured for long recording

### WaveForms Long-Term Logging Setup

1. Open **Scope** instrument
2. Configure channel (200 mV/div, DC coupling)
3. Go to **Logging** tab (or File → Logger)
4. Settings:
   - Sample interval: **10 seconds**
   - Duration: **110 minutes** (extra margin)
   - Log: **Channel 1 Average**
5. Click **Start**
6. Let it run unattended

### During Test Monitoring

| Time (min) | Scope Avg (mV) | Current (mA) | Battery V (optional) |
|------------|----------------|--------------|----------------------|
| 0 | | | |
| 10 | | | |
| 20 | | | |
| 30 | | | |
| 40 | | | |
| 50 | | | |
| 60 | | | |
| 70 | | | |
| 80 | | | |
| 90 | | | |
| **100** | | | **Must be >6V** |

### Post-Test Verification

1. **Stop logging** — export CSV
2. **Measure final battery voltage:** ______V
3. **Pass criteria:** Final voltage > 6.0V

| Result | Final Voltage | Status |
|--------|---------------|--------|
| ✅ PASS | >6.0V | Meets requirement |
| ⚠️ MARGINAL | 6.0-6.2V | Consider reducing power |
| ❌ FAIL | <6.0V | Must reduce system current |

---

## TX Coil Power Calculation

Once you know the available current for the coil:

### Power Available

$$P_{coil} = V_{supply} \times I_{available}$$

Example with 30mA available at 9V: $$P_{coil} = 9V \times 30mA = 270mW$$

### Coil Current vs Detection Range

|Coil Current|Power @ 9V|Expected Detection|
|---|---|---|
|10 mA|90 mW|~30mm|
|20 mA|180 mW|~40mm|
|30 mA|270 mW|~50mm|
|50 mA|450 mW|~60mm|

> **Requirement:** Detect iron (15mm radius, 50mm length) at 50mm depth

---

## Troubleshooting

### AD3 Power Supply Method

| Problem | Solution |
|---------|----------|
| I+ shows 0 mA | Check V+ connected to Arduino 5V pin |
| Arduino won't power on | Verify Master Enable ON, V+ = 5.0V |
| "Overcurrent" shutdown | Disconnect TX coil — exceeds 250mA |
| Current fluctuates | Normal; average over 10+ seconds |

### Shunt Resistor Method

| Problem | Solution |
|---------|----------|
| No scope reading | Check shunt resistor connections |
| Reading is 0 | Verify battery is connected and charged |
| Very noisy signal | Add 100nF capacitor across shunt |
| Reading is positive/negative | Polarity depends on probe orientation; use absolute value |
| Arduino won't start | Battery too weak; measure voltage directly |

### Both Methods

| Problem | Solution |
|---------|----------|
| OLED not working | Check I2C: SDA=Pin 20, SCL=Pin 21, Address=0x3C or 0x3D |
| No serial output | Verify baud rate 9600, check USB connection for monitoring |
| Higher than expected current | Check for shorts, verify no USB connected during test |

---

## Expected Values Summary

| Component | Typical Current |
|-----------|-----------------|
| Arduino Mega (running) | 50-80 mA |
| SSD1306 OLED | 10-20 mA |
| CPU load (prime calculations) | 0-5 mA |
| **Test program total** | **60-105 mA** |

### Power Budget Check

| Budget Item | Value |
|-------------|-------|
| Maximum for 100 min runtime | 120 mA |
| Your measured current | ______ mA |
| **Margin** | ______ mA |

---

## If Over Budget

Options to reduce power consumption:

### Software

- [ ] Increase delay between display updates (currently 500ms)
- [ ] Dim OLED display
- [ ] Sleep MCU between calculations

### Hardware

- [ ] Use Arduino Pro Mini (3.3V, 8MHz) - ~5mA
- [ ] Use ATmega328P standalone (no USB chip) - ~15mA
- [ ] Use more efficient voltage regulator

### Measured Savings

|Optimization|Current Saved|
|---|---|
|OLED 50% brightness|~10 mA|
|Remove USB chip (standalone)|~20 mA|
|8MHz instead of 16MHz|~10 mA|

---

## Related Notes

- [[Theory References#Electronics|Electronics Theory]]
- [[KiCad/README|PCB Design]]
- [[LTspice/README|Circuit Simulations]]
- [[Literature/duracell_9volt.pdf|Battery Datasheet]]

---

#power #measurement #battery #requirements #analog-discovery #lab
