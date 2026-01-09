# Metal Detector Test Guide - Analog Discovery 3

## Purpose

Test the metal detector firmware without coils by using the Analog Discovery 3 to:
1. **Measure** the TX output signal on Pin D9
2. **Generate** a test signal into Pin A0 (simulating RX coil)
3. **Verify** the DFT processing works correctly

---

## Equipment Required

| Item | Notes |
|------|-------|
| Arduino Nano | Programmed with minimal test firmware |
| Analog Discovery 3 | With WaveForms software |
| Breadboard | With Nano and OLED connected |
| Jumper wires | For AD3 connections |

---

## AD3 Pin Reference

```
Analog Discovery 3 Connector:
┌─────────────────────────────────────┐
│  1+  1-  2+  2-  W1  W2  T1  T2 ... │
│  ●   ●   ●   ●   ●   ●   ●   ●      │
│  Scope Ch1  Scope Ch2  Wavegen      │
│                                     │
│  GND GND GND ...                    │
│  ●   ●   ●                          │
└─────────────────────────────────────┘

1+/1- = Oscilloscope Channel 1 (differential)
2+/2- = Oscilloscope Channel 2 (differential)
W1    = Waveform Generator 1 output
W2    = Waveform Generator 2 output
GND   = Ground (multiple pins available)
```

---

## Test Setup Wiring

### Connections

| AD3 Pin | Arduino Pin | Purpose |
|---------|-------------|---------|
| **1+** (orange) | D9 | Measure TX output |
| **1-** (orange/white) | GND | Scope Ch1 reference |
| **W1** (yellow) | A0 | Inject test signal |
| **GND** (black) | GND | Common ground |

### Wiring Diagram

```
    ANALOG DISCOVERY 3                    ARDUINO NANO
    ┌──────────────────┐                  ┌──────────────┐
    │                  │                  │              │
    │  1+ (orange) ────┼──────────────────┼── D9 (TX)    │
    │  1- (org/wht) ───┼────┐             │              │
    │                  │    │             │              │
    │  W1 (yellow) ────┼────┼─────────────┼── A0 (RX)    │
    │                  │    │             │              │
    │  GND (black) ────┼────┴─────────────┼── GND        │
    │                  │                  │              │
    └──────────────────┘                  └──────────────┘
```

> **Important:** Always connect GND first before signal wires.

---

## Test 1: Verify TX Output (2 kHz Square Wave)

### Objective
Confirm that Pin D9 outputs a 2 kHz square wave.

### WaveForms Setup

1. Open **WaveForms** software
2. Click **Scope** to open the oscilloscope

### Scope Settings

| Parameter | Value |
|-----------|-------|
| Channel 1 | Enabled |
| Coupling | DC |
| Range | 0-5V (or Auto) |
| Time/Div | 200 µs |
| Trigger | Ch1, Rising edge, 2.5V |

### Procedure

1. Power on the Arduino Nano
2. Click **Run** in WaveForms Scope
3. Observe the waveform

### Expected Result

```
     5V ┤ ┌───┐   ┌───┐   ┌───┐   ┌───┐
        │ │   │   │   │   │   │   │   │
        │ │   │   │   │   │   │   │   │
     0V ┤─┘   └───┘   └───┘   └───┘   └───
        └─────────────────────────────────
          |<-250µs->|
          |<---500µs (one period)--->|
```

| Measurement | Expected Value | Tolerance |
|-------------|----------------|-----------|
| Frequency | 2.000 kHz | ±1% |
| Period | 500 µs | ±5 µs |
| Duty Cycle | 50% | ±2% |
| Amplitude | 5V (0 to 5V) | |

### Use Measurements

In WaveForms Scope:
1. Click **View** → **Measurements**
2. Add: Frequency, Period, Duty Cycle, Amplitude

---

## Test 2: Inject Test Signal and Verify DFT

### Objective
Send a known 2 kHz sine wave into A0 and verify the DFT calculates correct magnitude and phase.

### WaveForms Setup

1. Open **Wavegen** tool
2. Configure Waveform Generator 1 (W1)

### Wavegen Settings

| Parameter | Value | Notes |
|-----------|-------|-------|
| Channel | W1 | Yellow wire |
| Type | Sine | Start with sine wave |
| Frequency | 2000 Hz | Match TX frequency |
| Amplitude | 2 V | Peak-to-peak |
| Offset | 2.5 V | Center at mid-scale ADC |
| Symmetry | 50% | Normal sine |

This produces a signal from **1.5V to 3.5V**, centered at 2.5V (ADC value ~512).

### Procedure

1. Configure Wavegen as above
2. Click **Run** on Wavegen
3. Observe the OLED display on Arduino

### Expected Display Output

Press the **D4 button** to toggle between two screens:

**Screen 1: DFT Results**
```
=== DFT ===

Re:   <non-zero>
Im:   <non-zero>

Mag:  <value>
Phase: <angle> deg
```

**Screen 2: Debug Info**
```
=== DEBUG ===

ADC:  512        (should fluctuate with signal)
Min:  <low val>  (lowest ADC seen)
Max:  <high val> (highest ADC seen)
TX:   HIGH/LOW   (toggles at 2kHz)
DFT#: <count>    (increments ~125/sec)
Vpp:  <range>    (Max - Min, shows signal swing)
```

### Understanding the Results

With a 2 kHz sine wave input synchronized to TX:

| Display Value | Expected Behavior |
|---------------|-------------------|
| **Re** | Non-zero, varies with phase alignment |
| **Im** | Non-zero, varies with phase alignment |
| **Mag** | Proportional to input amplitude (~60-120 for 2Vpp) |
| **Phase** | Depends on phase relationship to TX |

---

## Test 3: Phase Behavior (Important!)

### Understanding Phase Drift

When testing with the AD3 wavegen, you may notice the phase reading is "all over the place" even with correct wiring. **This is expected behavior!**

**Root Cause:** The AD3 wavegen and Arduino run from independent crystal oscillators. Even when both are set to 2000 Hz, they will have slightly different actual frequencies:

| Source | Set Frequency | Actual Frequency |
|--------|---------------|------------------|
| Arduino TX | 2000 Hz | ~2001.8 Hz |
| AD3 Wavegen | 2000 Hz | ~2001.6 Hz |

A difference of just 0.2 Hz causes the phase to rotate through 360° every 5 seconds!

**This confirms your phase detection is working correctly** - it's accurately tracking the changing phase relationship between two unsynchronized signals.

### Method 1: Verify Phase Rotation (Confirms System Works)

1. Set Wavegen to exactly **2000 Hz**
2. Watch the Phase value on the display
3. **Expected:** Phase slowly rotates through -180° to +180°
4. Rotation speed depends on frequency mismatch (~5-10 sec per cycle typical)

If phase rotates smoothly: **PASS** - phase detection is working!

### Method 2: Intentional Frequency Offset

1. Set Wavegen to **2001 Hz** (1 Hz offset)
2. Phase should rotate through 360° every 1 second
3. Set Wavegen to **2010 Hz** (10 Hz offset)
4. Phase should rotate through 360° every 0.1 seconds (appears random)

### Method 3: Synchronized Testing (Stable Phase)

To get a stable phase reading, you must synchronize the AD3 to the Arduino:

**Option A: External Trigger**
1. Connect Arduino D9 (TX) to AD3 Trigger input (T1)
2. In WaveForms Wavegen, set trigger source to "External 1"
3. This syncs wavegen output to Arduino timing

**Option B: Use Arduino TX as Reference**
1. Connect a wire from D9 to a second AD3 scope channel
2. In Scope, trigger on the TX signal
3. Observe that the injected signal phase is stable relative to TX

### Why This Matters for Metal Detection

In the real metal detector:
- TX coil creates the magnetic field
- RX coil receives the response
- Both signals come from the SAME physical system
- Phase is naturally synchronized (determined by metal properties)

The AD3 test setup has independent clocks, so phase drifts. This is a limitation of the test setup, not the firmware.

---

## Test 4: Amplitude Test

### Objective
Verify magnitude scales with input amplitude.

### Procedure

1. Set Wavegen frequency to 2000 Hz, offset 2.5V
2. Record magnitude for different amplitudes:

| Wavegen Amplitude (Vpp) | Signal Range | Expected Trend |
|-------------------------|--------------|----------------|
| 0.5 V | 2.25V - 2.75V | Low Mag |
| 1.0 V | 2.0V - 3.0V | Medium Mag |
| 2.0 V | 1.5V - 3.5V | High Mag |
| 4.0 V | 0.5V - 4.5V | Very High Mag |

> **Warning:** Do not exceed 5V total (offset + amplitude/2 < 5V)

### Expected Result

Magnitude should increase approximately linearly with input amplitude.

---

## Test 5: Frequency Response

### Objective
Verify the DFT only responds strongly to the 2 kHz signal.

### Procedure

1. Set Wavegen amplitude to 2Vpp, offset 2.5V
2. Sweep frequency and record magnitude:

| Frequency | Expected Magnitude |
|-----------|--------------------|
| 500 Hz | Very low (near zero) |
| 1000 Hz | Low |
| 1500 Hz | Low |
| **2000 Hz** | **Maximum** |
| 2500 Hz | Low |
| 3000 Hz | Low |
| 4000 Hz | Very low |

### Expected Result

The DFT should show a strong peak at 2 kHz and attenuate other frequencies. This is the "lock-in" effect of the single-bin DFT.

---

## Test 6: Noise Rejection

### Objective
Verify the DFT rejects signals not at 2 kHz.

### Procedure

1. Set Wavegen to **1000 Hz** sine wave, 2Vpp, 2.5V offset
2. Observe magnitude (should be low)
3. Change to **2000 Hz**
4. Observe magnitude (should be high)

This demonstrates the narrow-band filtering effect of the DFT.

---

## Using the Debug Screen

The debug screen provides quick verification of each subsystem:

### TX Verification
- **TX:** Should alternate between HIGH and LOW
- If stuck on one value: Timer0 ISR not running (check sei())

### ADC Verification
- **ADC:** Should show values around 512 with no signal (floating input)
- With Wavegen signal: Should fluctuate around the offset
- **Min/Max:** Shows the range of ADC values seen
- **Vpp:** Peak-to-peak swing - with 2Vpp input centered at 2.5V:
  - Expected Vpp ≈ 410 (2V / 5V × 1024)

### DFT Verification
- **DFT#:** Should increment continuously (~125 times per second)
- If stuck at 0: ADC ISR not running
- If incrementing: DFT accumulation working

### Quick Health Check

| Debug Value | Healthy | Problem |
|-------------|---------|---------|
| TX | Alternates HIGH/LOW | Stuck = no interrupts |
| ADC | ~512 (no signal) | 0 or 1023 = wiring issue |
| Min/Max | Updates with signal | Stuck = ADC not sampling |
| DFT# | Incrementing | Stuck = ISR not running |
| Vpp | Matches input signal | 0 = no AC signal |

---

## Test Summary Checklist

| Test | Pass Criteria | Result |
|------|--------------|--------|
| TX Output | 2 kHz ±1%, 50% duty, 5V amplitude | [ ] |
| DFT Basic | Non-zero Re, Im, Mag with 2kHz input | [ ] |
| Phase Detection | Phase rotates smoothly (not stuck) | [ ] |
| Phase Rotation Speed | ~1 sec/cycle at 1Hz offset | [ ] |
| Amplitude Scaling | Mag increases with input amplitude | [ ] |
| Frequency Selectivity | Mag peaks at 2kHz, low elsewhere | [ ] |
| Noise Rejection | Low Mag for non-2kHz signals | [ ] |

> **Note:** Phase appearing "random" with 2kHz wavegen input is NORMAL and indicates
> the phase detection is working. See Test 3 for full explanation.

---

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| No TX signal | Firmware not running | Check upload, verify sei() called |
| TX wrong frequency | Timer0 config wrong | Verify OCR0A = 249, prescaler = 8 |
| Mag always 0 | No signal on A0 | Check W1 connection, verify Wavegen running |
| Mag always same | DC only, no AC | Check Wavegen offset centered at 2.5V |
| **Phase "all over the place"** | **Normal! Independent clocks** | **See Test 3 - this confirms phase works!** |
| Phase stuck at one value | No AC signal | Check wavegen is running, amplitude > 0 |
| Display not updating | DFT_done not set | Check ADC ISR running |

---

## Quick Reference: WaveForms Settings

### Scope (Measuring TX)
```
Channel 1: ON, DC coupling
Range: 5V
Time: 200 µs/div
Trigger: Ch1, Rising, 2.5V
```

### Wavegen (Simulating RX)
```
W1: Sine
Frequency: 2000 Hz
Amplitude: 2 V (peak-to-peak)
Offset: 2.5 V
```

---

## Notes

- The AD3 Wavegen output impedance is 50Ω, which is fine for the high-impedance ADC input
- Keep wires short to minimize noise pickup
- If using both scope channels, connect Ch2 to A0 to see the injected signal
- The 8 kHz sampling creates some aliasing - only 2 kHz signals will be correctly measured
