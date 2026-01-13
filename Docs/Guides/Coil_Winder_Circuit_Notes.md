# Coil Winder - Circuit Build Notes

> [!abstract] Document Purpose
> Detailed circuit build notes, debugging findings, and design considerations for the coil winder electronics. Documents lessons learned during prototyping.

---

## 1. Break-Beam Optical Sensor (Recommended)

### 1.1 Overview

A DIY slotted optical sensor built from discrete components available at DTU. This **break-beam** design is more reliable than the reflective ITR8307 because:

- No reflective surface required - detects a slot/hole in an encoder disc
- Binary detection: beam either passes or is blocked
- Less sensitive to dirt, dust, and alignment
- Clean digital output via LM311 comparator

**Components (all from DTU):**
| Component | Part Number | Function |
|-----------|-------------|----------|
| IR Emitter | SFH4546 | 940nm infrared LED |
| IR Photodiode | BP104 or SFH309-4 | Infrared detector |
| Comparator | LM311 | Analog to digital conversion |
| Resistors | 220Ω, 10kΩ, 47kΩ (×2) | Current limit, pull-up, reference |
| Capacitor | 100nF | Noise filtering |

### 1.2 Circuit Schematic

```
                    BREAK-BEAM OPTICAL SENSOR
                    ─────────────────────────

     ┌─────────────────────────────────────────────────────────────┐
     │                                                             │
     │      IR EMITTER                      IR DETECTOR            │
     │      (SFH4546)                       (BP104)                │
     │                                                             │
     │         5V                              5V                  │
     │          │                               │                  │
     │        [220Ω]                          [47kΩ] R3            │
     │          │                               │                  │
     │          ▼                               │                  │
     │      ┌───┴───┐        Encoder       ┌───┴───┐              │
     │      │  ───► │        Disc          │   ◄── │              │
     │      │ SFH   │◄─ ─ ─ ┌───┐ ─ ─ ─ ─ ►│  BP   │              │
     │      │ 4546  │   IR  │   │   IR     │  104  │              │
     │      │  ───► │  beam │ █ │  beam    │   ◄── │              │
     │      └───┬───┘       └───┘          └───┬───┘              │
     │          │           slot               │                  │
     │          ▼                              │                  │
     │         GND                             ├──────────────┐   │
     │                                         │              │   │
     │                                      [100nF]           │   │
     │                                         │              │   │
     │                                        GND             │   │
     │                                                        │   │
     │                              ┌──────────────────────────┘   │
     │                              │                              │
     │                              ▼                              │
     │                     ┌───────────────┐                       │
     │                     │    LM311      │                       │
     │                     │  COMPARATOR   │                       │
     │                     │               │                       │
     │             Vref ──►│ -   (pin 3)   │                       │
     │                     │               │                       │
     │        Photodiode──►│ +   (pin 2)   │                       │
     │            signal   │               │                       │
     │                     │      OUT ─────┼──┬──► To Arduino D2   │
     │                     │    (pin 7)    │  │    (INT0)          │
     │                     │               │ [10kΩ] R4             │
     │                     │   GND   V+    │  │                    │
     │                     └────┬─────┬────┘  │                    │
     │                          │     │       5V                   │
     │                         GND   5V                            │
     │                                                             │
     └─────────────────────────────────────────────────────────────┘

     REFERENCE VOLTAGE DIVIDER:

         5V
          │
        [47kΩ] R1
          │
          ├────────► LM311 pin 3 (inverting input)
          │
        [47kΩ] R2
          │
         GND

     Vref = 5V × (47k / (47k + 47k)) = 2.5V
```

### 1.3 Detailed Circuit Schematic

```
                                    5V
                                     │
          ┌──────────────────────────┼──────────────────────────────┐
          │                          │                              │
          │                          │                              │
        [220Ω]                     [47kΩ]                         [47kΩ]
         R_LED                       R1                             R3
          │                          │                              │
          │                          │    ┌───────────────┐         │
    ┌─────┴─────┐                    │    │    LM311      │         │
    │  SFH4546  │                    │    │               │         │
    │    (+)    │                    └───►│3 (-)         7├──┬──────┼───► D2
    │   LED     │                         │               │  │      │
    │    (-)    │                    ┌───►│2 (+)    GND  4├──┼──┐   │
    └─────┬─────┘                    │    │               │  │  │   │
          │                          │    │         V+   8├──┼──┼───┤
         GND                         │    └───────────────┘  │  │   │
                                     │                       │  │   │
                                     │                    [10kΩ] │   │
    ┌─────────────┐                  │                     R4 │   │
    │   BP104     │                  │                       │  │   │
    │  Cathode(K) ├──────────────────┼───────────────────────┘  │   │
    │             │                  │                          │   │
    │   Anode(A)  ├──────────────────┘                          │   │
    └─────────────┘                                             │   │
          │                                                     │   │
          │                                                    GND  │
        [100nF]                                                     │
          │                        [47kΩ]                           │
         GND                         R2                             │
                                     │                              │
                                    GND                            5V


    Component List (from DTU Component Shop):
    ─────────────────────────────────────────
    SFH4546      - IR Emitter (940nm)
    BP104        - IR Photodiode (alternative: SFH309-4)
    LM311        - Differential Comparator
    R_LED: 220Ω  - LED current limiter (220R from E96)
    R1, R2: 47kΩ - Reference voltage divider (47K0 from E96)
    R3: 47kΩ     - Photodiode load resistor
    R4: 10kΩ     - Comparator output pull-up (10K0 from E96)
    C1: 100nF    - Noise filter capacitor (100n Ceramic)
```

### 1.4 How It Works

**Normal operation (beam passes through slot):**
1. SFH4546 emits IR light continuously
2. IR passes through slot in encoder disc
3. BP104 receives IR → conducts → pulls signal LOW (~0.5V)
4. LM311: (+) input < Vref → output goes HIGH (pulled up by R4)

**Beam blocked (solid part of disc):**
1. SFH4546 emits IR light continuously
2. Encoder disc blocks IR beam
3. BP104 receives no IR → no conduction → signal HIGH (~5V)
4. LM311: (+) input > Vref → output goes LOW (open collector sinks)

**Interrupt trigger:**
- Configure INT0 for **falling edge** (HIGH→LOW)
- Each slot passing triggers one interrupt = one turn counted

### 1.5 Encoder Disc Design

```
    SLOTTED ENCODER DISC (for break-beam sensor)

              ┌─────────────────────────────┐
              │                             │
              │      ████████████████       │    Solid material
              │     ██              ██      │    (blocks IR beam)
              │    ██                ██     │
              │   ██    ┌──────┐     ██    │
              │   ██    │      │     ██    │    Slot/hole
              │   ██    │ SLOT │     ██    │    (IR passes through)
              │   ██    │      │     ██    │
              │   ██    └──────┘     ██    │
              │    ██                ██     │
              │     ██      ●       ██      │    Center hole (M8)
              │      ████████████████       │
              │                             │
              └─────────────────────────────┘

    Dimensions:
    - Disc diameter: 40-50mm
    - Slot width: 5-8mm (wide enough for reliable detection)
    - Slot radial length: 10-15mm
    - Slot position: at outer edge for max angular resolution
    - Material: 3D printed PLA (opaque)
    - Thickness: 2-3mm
```

**Alternative: Multi-slot disc for higher resolution:**
```
    4-SLOT ENCODER (optional - 4 pulses per revolution)

              ┌─────────────────────────────┐
              │    ██   ═══   ██   ═══      │
              │  ██               ══  ██    │
              │ ║                      ║    │    4 slots at 90°
              │ ║         ●            ║    │    intervals
              │ ║                      ║    │
              │  ██               ══  ██    │
              │    ██   ═══   ██   ═══      │
              └─────────────────────────────┘

    Use 4 slots if you need:
    - Finer position tracking
    - Direction detection (with 2 sensors)
    - Divide count by 4 in firmware
```

### 1.6 Sensor Mounting

```
    SIDE VIEW - SENSOR MOUNT

         Encoder disc
              │
              ▼
         ┌─────────┐
    ─────┤   ███   ├─────  ◄── Disc passes through gap
         └────┬────┘
              │
       ┌──────┴──────┐
       │             │
    ┌──┴──┐       ┌──┴──┐
    │SFH  │       │BP   │
    │4546 │  5mm  │104  │      Gap: 5-10mm
    │ IR  │◄─────►│Photo│      (disc must fit + clearance)
    │ LED │       │diode│
    └──┬──┘       └──┬──┘
       │             │
    ───┴─────────────┴───  ◄── Mounting bracket (3D printed)
```

### 1.7 Component Values Calculation

**IR LED Current (SFH4546):**
```
I_LED = (Vcc - Vf) / R_LED
      = (5V - 1.4V) / 220Ω
      = 16.4 mA

This is within safe operating range (max 100mA continuous).
Increase R_LED to 330Ω for longer battery life (10.9 mA).
```

**Photodiode Load Resistor (R3):**
```
Higher R3 = more sensitivity but slower response
47kΩ provides good balance for hand-crank speeds.

At 300 RPM = 5 Hz, rise/fall time is not critical.
```

**Reference Voltage:**
```
Vref = Vcc × R2 / (R1 + R2)
     = 5V × 47k / (47k + 47k)
     = 2.5V (midpoint)

Adjust R1/R2 ratio if detection is unreliable:
- Lower Vref (e.g., 2V): more sensitive to weak signals
- Higher Vref (e.g., 3V): more noise immunity
```

### 1.8 LM311 Pinout

```
    LM311 (DIP-8 package)

           ┌───────┐
    GND  1─┤       ├─8  V+
    IN+  2─┤ LM311 ├─7  OUT (open collector)
    IN-  3─┤       ├─6  BAL/STROBE (not used)
    V-   4─┤       ├─5  BAL (not used)
           └───────┘

    Connections:
    - Pin 1 (GND): Connect to ground
    - Pin 2 (IN+): Photodiode signal
    - Pin 3 (IN-): Reference voltage (2.5V)
    - Pin 4 (V-):  Connect to ground (single supply)
    - Pin 7 (OUT): To Arduino D2 via 10kΩ pull-up
    - Pin 8 (V+):  Connect to 5V
    - Pins 5,6:    Leave unconnected or tie to V-
```

---

## 2. ITR8307 Optical Sensor (Legacy/Alternative)

### 2.1 Sensor Type Clarification

**IMPORTANT:** The ITR8307/F43 is a **REFLECTIVE** sensor, not a slot/interrupter sensor!

- Both IR LED and phototransistor point in the **same direction**
- Detects IR light bouncing back from a reflective surface
- Optimal detection distance: **1-3mm**
- Works best with **white/reflective surfaces**

### 2.2 Correct Pinout (from datasheet ITR8307-F43.pdf)

```
    Physical Layout (looking at component side):

    LED side        Detector side
    ┌──┐            ┌──┐
    │  │    gap     │  │
    │IR│◄──────────►│PT│    Both point UPWARD (same direction)
    └┬┬┘            └┬┬┘
     12              34

    Pin 1 = CATHODE  (LED -)     ──► GND
    Pin 2 = ANODE    (LED +)     ──► 330Ω ──► 5V
    Pin 3 = COLLECTOR (Detector) ──► 10kΩ to 5V + D2
    Pin 4 = EMITTER  (Detector -) ──► GND
```

### 2.3 Circuit Schematic

```
                         5V
                          │
            ┌─────────────┼─────────────┐
            │             │             │
          [330Ω]        [10kΩ]       [100nF]
            │             │             │
            │             ├─────────────┼──► D2 (INT0)
            │             │             │
    Pin 2 ──┘    Pin 3 ───┘             │
    (Anode)      (Collector)            │
                                        │
    Pin 1 ──┬── Pin 4 ──────────────────┴──► GND
    (Cathode)   (Emitter)
```

**Component Values:**
- R1 = 330Ω (LED current limit): (5V - 1.2V) / 330Ω ≈ 11.5mA
- R2 = 10kΩ (pull-up resistor)
- C1 = 100nF (noise filter) - **essential!**

### 2.4 Encoder Wheel Design for Reflective Sensor

Since this is a reflective sensor, the encoder wheel needs a **reflective stripe**:

```
    ┌─────────────────┐
    │  ████████████   │    Black/dark surface
    │  ████████████   │    (non-reflective)
    │                 │
    │   ┌───────┐     │
    │   │ WHITE │     │    White/reflective stripe
    │   │STRIPE │     │    (reflects IR back to detector)
    │   └───────┘     │
    │        ●        │    Center hole (M8)
    └─────────────────┘

    Alternative: White wheel with black stripe
    (inverted logic - adjust interrupt edge)
```

**Stripe dimensions:**
- Width: ~5mm
- Arc length: ~10-15mm
- Material: White tape, white paint, or white 3D printed insert

---

## 3. Servo Noise Issues

### 3.1 Problem Description

During testing, the optical sensor signal showed significant noise/oscillation when the servo was connected:

- Signal oscillating between 4.4V and 5.2V
- High-frequency ripple on power supply
- Sensor readings unreliable

**Root Cause:** Servo motors draw large current pulses during PWM cycles, causing voltage spikes and dips on the shared 5V supply.

### 3.2 Oscilloscope Observations

```
    Without servo connected:
    5.0V ────────────────────────── Clean, stable signal

    With servo connected (no filtering):
    5.2V ┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
         │ │ │ │ │ │ │ │ │ │ │      High-frequency noise
    4.4V ┘ └─┘ └─┘ └─┘ └─┘ └─┘      from servo PWM
```

### 3.3 Solution: Power Supply Filtering

**Required capacitors:**

1. **100µF electrolytic** near servo power:
   - Absorbs large current spikes
   - Place as close to servo connector as possible

2. **100nF ceramic** near sensor:
   - Filters high-frequency noise
   - Place between Pin 3 and GND

```
    POWER DISTRIBUTION (Filtered)

    5V Rail ──┬───────────────────────┬──────────────────┐
              │                       │                  │
           ┌──┴──┐                 ┌──┴──┐            ┌──┴──┐
           │100µF│                 │100nF│            │100nF│
           │     │                 │     │            │     │
           └──┬──┘                 └──┬──┘            └──┬──┘
              │                       │                  │
    GND ──────┴───────────────────────┴──────────────────┘
              │                       │
           Servo                   Sensor
```

### 3.4 Additional Noise Reduction Techniques

1. **Separate power paths:** Run servo power directly from Arduino 5V pin, sensor from a different 5V source if available

2. **Twisted pair wiring:** Twist sensor signal wire with its ground wire

3. **Shorter wires:** Keep sensor wires as short as possible

4. **Shielding:** If noise persists, use shielded cable for sensor

---

## 4. Code Review Findings

### 4.1 Button State Tracking Bug

**Problem:** Buttons would occasionally stop responding, requiring Arduino reset.

**Root Cause:** In the original `buttons_poll()` function, the previous button state (`g_btn_prev`) was only updated at the end of the function. When the debounce timer blocked early return, `g_btn_prev` was never updated, causing state desynchronization.

**Original problematic code:**
```c
uint8_t buttons_poll(void)
{
    // ...
    if (now - g_last_time < DEBOUNCE_MS) {
        return 0;  // Early return - g_btn_prev not updated!
    }
    // ... button detection logic ...
    g_btn_prev = btn_state;  // Only updated if we get here
    return events;
}
```

**Fixed code:**
```c
uint8_t buttons_poll(void)
{
    // ...
    uint8_t changed = btn_state ^ g_btn_prev;
    g_btn_prev = btn_state;  // Always update state first!

    if (now - g_last_time < DEBOUNCE_MS) {
        return 0;
    }
    // ... detect buttons using 'changed' variable ...
}
```

### 4.2 Encoder Noise Filtering

**Problem:** Noisy sensor signal caused false triggers and incorrect turn counts.

**Solution:** Added software debouncing in `encoder_check_turn()`:

```c
#define ENCODER_DEBOUNCE_MS  20  // Minimum time between valid triggers

bool encoder_check_turn(void)
{
    if (!g_new_turn) return false;

    uint32_t now = millis();
    if (now - g_last_trigger < ENCODER_DEBOUNCE_MS) {
        // Too fast - likely noise
        g_new_turn = false;
        cli();
        if (g_turn_count > 0) g_turn_count--;  // Undo false count
        sei();
        return false;
    }

    g_last_trigger = now;
    g_new_turn = false;
    return true;
}
```

**Debounce time selection:**
- 20ms allows up to 50 triggers/second
- At 60 RPM (1 turn/sec), plenty of margin
- At 300 RPM (5 turns/sec), still OK (200ms between turns)
- Increase if still getting false triggers
- Decrease if missing fast rotations

---

## 5. Firmware Module Structure

### 5.1 File Organization

```
coil_winder/src/
├── config.h         - Pin definitions, timing constants
├── timer.h/.c       - Millisecond timer (Timer0)
├── encoder.h/.c     - Turn counting (INT0 interrupt)
├── servo.h/.c       - PWM servo control (Timer1)
├── buttons.h/.c     - Button input with debouncing
├── buzzer.h/.c      - Buzzer output control
├── twi.h/.c         - I2C/TWI communication
├── ssd1306.h/.c     - OLED driver with 5x7 font
├── coil_presets.h/.c- Coil presets + winder state
└── main.c           - Application logic (~200 lines)
```

### 5.2 Key Configuration (config.h)

```c
/* Pin Definitions */
#define ENCODER_PIN     PD2     // INT0
#define SERVO_PIN       PB1     // OC1A (D9)
#define BTN_RESET_PIN   PD3     // D3
#define BTN_PRESET_PIN  PD4     // D4
#define BTN_START_PIN   PD5     // D5
#define BUZZER_PIN      PB2     // D10

/* Timing */
#define DEBOUNCE_MS         50   // Button debounce
#define DISPLAY_UPDATE_MS   100  // 10Hz display refresh

/* Servo */
#define SERVO_CENTER    90   // Center angle (degrees)
#define SERVO_RANGE     25   // Range each side
```

### 5.3 Memory Usage

```
RAM:   [========  ]  77.4% (1585 / 2048 bytes)
Flash: [=         ]  11.6% (3740 / 32256 bytes)
```

Plenty of headroom for additional features.

---

## 6. Testing Checklist

### 6.1 Component Test Sequence

- [ ] **Power supply:** 5V stable, no shorts
- [ ] **OLED display:** Shows startup screen
- [ ] **Buzzer:** Beeps on power-up
- [ ] **Buttons:** Each button triggers correct action
- [ ] **Servo:** Moves to center on startup
- [ ] **Sensor (no servo):** Clean 5V signal, drops to ~0V with white paper
- [ ] **Sensor (with servo):** Signal still clean after adding capacitors
- [ ] **Full system:** Turn counting works, servo traverses

### 6.2 Oscilloscope Test Points

| Test Point | Expected | Problem If... |
|------------|----------|---------------|
| D2 (idle) | ~5V stable | Oscillating = noise |
| D2 (triggered) | Clean 5V→0V→5V | Slow rise = weak pull-up |
| D9 (servo) | 1-2ms pulses at 50Hz | Missing = Timer1 issue |
| 5V rail | 5.0V ± 0.1V | Dips = add capacitance |

### 6.3 Debugging with Analog Discovery

The AD3 is excellent for debugging:

1. **Scope mode:** Check signal integrity on D2
2. **Logic analyzer:** Verify interrupt timing
3. **Power analysis:** Monitor 5V rail during servo operation

---

## 7. Common Issues and Solutions

| Issue | Symptom | Solution |
|-------|---------|----------|
| Sensor hot | Component warm to touch | Check LED polarity (Pin 1=Cathode→GND) |
| No detection | Always 5V or always 0V | Verify reflective surface at 1-3mm |
| Double counts | Count jumps by 2+ | Add 100nF cap, increase debounce time |
| Servo jitter | Servo twitches randomly | Add 100µF cap on servo power |
| Buttons stuck | Stop responding | Upload fixed firmware (v2) |
| Display noise | Flickering/artifacts | Check I2C connections, add pull-ups |

---

## 8. Version History

| Version | Date | Changes |
|---------|------|---------|
| v1.0 | 2026-01-12 | Initial firmware, monolithic main.c |
| v1.1 | 2026-01-12 | Modular code structure |
| v2.0 | 2026-01-12 | Fixed button state bug, added encoder debounce |
| v2.1 | 2026-01-13 | New break-beam sensor design (SFH4546 + BP104 + LM311) |

---

## 9. References

- ITR8307/F43 Datasheet (Everlight) - Legacy sensor
- SFH4546 Datasheet (OSRAM) - IR Emitter
- BP104 Datasheet (Vishay) - IR Photodiode
- LM311 Datasheet (Texas Instruments) - Comparator
- ATmega328P Datasheet (Section 13: External Interrupts)
- SSD1306 Datasheet (Solomon Systech)

---

*Circuit build notes for DTU 34621 Metal Detector - Coil Winder*
*Last updated: 2026-01-13 - Added break-beam optical sensor design*
