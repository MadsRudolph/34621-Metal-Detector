# Coil Winder - Circuit Build Notes

> [!abstract] Document Purpose
> Detailed circuit build notes, debugging findings, and design considerations for the coil winder electronics. Documents lessons learned during prototyping.

---

## 1. ITR8307 Optical Sensor

### 1.1 Sensor Type Clarification

**IMPORTANT:** The ITR8307/F43 is a **REFLECTIVE** sensor, not a slot/interrupter sensor!

- Both IR LED and phototransistor point in the **same direction**
- Detects IR light bouncing back from a reflective surface
- Optimal detection distance: **1-3mm**
- Works best with **white/reflective surfaces**

### 1.2 Correct Pinout (from datasheet ITR8307-F43.pdf)

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

### 1.3 Circuit Schematic

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

### 1.4 Encoder Wheel Design for Reflective Sensor

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

## 2. Servo Noise Issues

### 2.1 Problem Description

During testing, the optical sensor signal showed significant noise/oscillation when the servo was connected:

- Signal oscillating between 4.4V and 5.2V
- High-frequency ripple on power supply
- Sensor readings unreliable

**Root Cause:** Servo motors draw large current pulses during PWM cycles, causing voltage spikes and dips on the shared 5V supply.

### 2.2 Oscilloscope Observations

```
    Without servo connected:
    5.0V ────────────────────────── Clean, stable signal

    With servo connected (no filtering):
    5.2V ┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐
         │ │ │ │ │ │ │ │ │ │ │      High-frequency noise
    4.4V ┘ └─┘ └─┘ └─┘ └─┘ └─┘      from servo PWM
```

### 2.3 Solution: Power Supply Filtering

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

### 2.4 Additional Noise Reduction Techniques

1. **Separate power paths:** Run servo power directly from Arduino 5V pin, sensor from a different 5V source if available

2. **Twisted pair wiring:** Twist sensor signal wire with its ground wire

3. **Shorter wires:** Keep sensor wires as short as possible

4. **Shielding:** If noise persists, use shielded cable for sensor

---

## 3. Code Review Findings

### 3.1 Button State Tracking Bug

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

### 3.2 Encoder Noise Filtering

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

## 4. Firmware Module Structure

### 4.1 File Organization

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

### 4.2 Key Configuration (config.h)

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

### 4.3 Memory Usage

```
RAM:   [========  ]  77.4% (1585 / 2048 bytes)
Flash: [=         ]  11.6% (3740 / 32256 bytes)
```

Plenty of headroom for additional features.

---

## 5. Testing Checklist

### 5.1 Component Test Sequence

- [ ] **Power supply:** 5V stable, no shorts
- [ ] **OLED display:** Shows startup screen
- [ ] **Buzzer:** Beeps on power-up
- [ ] **Buttons:** Each button triggers correct action
- [ ] **Servo:** Moves to center on startup
- [ ] **Sensor (no servo):** Clean 5V signal, drops to ~0V with white paper
- [ ] **Sensor (with servo):** Signal still clean after adding capacitors
- [ ] **Full system:** Turn counting works, servo traverses

### 5.2 Oscilloscope Test Points

| Test Point | Expected | Problem If... |
|------------|----------|---------------|
| D2 (idle) | ~5V stable | Oscillating = noise |
| D2 (triggered) | Clean 5V→0V→5V | Slow rise = weak pull-up |
| D9 (servo) | 1-2ms pulses at 50Hz | Missing = Timer1 issue |
| 5V rail | 5.0V ± 0.1V | Dips = add capacitance |

### 5.3 Debugging with Analog Discovery

The AD3 is excellent for debugging:

1. **Scope mode:** Check signal integrity on D2
2. **Logic analyzer:** Verify interrupt timing
3. **Power analysis:** Monitor 5V rail during servo operation

---

## 6. Common Issues and Solutions

| Issue | Symptom | Solution |
|-------|---------|----------|
| Sensor hot | Component warm to touch | Check LED polarity (Pin 1=Cathode→GND) |
| No detection | Always 5V or always 0V | Verify reflective surface at 1-3mm |
| Double counts | Count jumps by 2+ | Add 100nF cap, increase debounce time |
| Servo jitter | Servo twitches randomly | Add 100µF cap on servo power |
| Buttons stuck | Stop responding | Upload fixed firmware (v2) |
| Display noise | Flickering/artifacts | Check I2C connections, add pull-ups |

---

## 7. Version History

| Version | Date | Changes |
|---------|------|---------|
| v1.0 | 2026-01-12 | Initial firmware, monolithic main.c |
| v1.1 | 2026-01-12 | Modular code structure |
| v2.0 | 2026-01-12 | Fixed button state bug, added encoder debounce |

---

## 8. References

- ITR8307/F43 Datasheet (Everlight)
- ATmega328P Datasheet (Section 13: External Interrupts)
- SSD1306 Datasheet (Solomon Systech)

---

*Circuit build notes for DTU 34621 Metal Detector - Coil Winder*
*Last updated: 2026-01-12*
