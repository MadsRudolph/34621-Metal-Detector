# Coil Design - VLF Metal Detector

## System Overview

Three-coil concentric design with multi-layer solenoid windings:

```
         ┌─────────────────────────────────────────┐
         │              TOP VIEW                   │
         │                                         │
         │    ┌─────────────────────────────┐     │
         │    │         TX COIL              │     │
         │    │        250 mm ⌀              │     │
         │    │    ┌───────────────────┐     │     │
         │    │    │   BUCKING COIL    │     │     │
         │    │    │       TBD         │     │     │
         │    │    │   ┌───────────┐   │     │     │
         │    │    │   │  RX COIL  │   │     │     │
         │    │    │   │    TBD    │   │     │     │
         │    │    │   └───────────┘   │     │     │
         │    │    └───────────────────┘     │     │
         │    └─────────────────────────────┘     │
         │                                         │
         └─────────────────────────────────────────┘
```

---

## Formula: Wheeler Multi-Layer Solenoid

$$L \text{ (µH)} = \frac{0.8 \times r^2 \times N^2}{6r + 9l + 10w}$$

Where (all in cm):
- $r$ = mean radius of coil
- $l$ = axial length (height of winding)
- $w$ = radial build (thickness of winding)
- $N$ = total number of turns

---

## TX Coil - READY TO IMPLEMENT ✓

### Specifications

| Parameter | Value |
|-----------|-------|
| Form diameter | **250 mm** |
| Wire diameter | **0.52 mm (AWG 24)** |
| Total turns | **105 turns** |
| Turns per layer | **35 turns** |
| Number of layers | **3 layers** |
| Axial length | **18 mm** |
| Radial build | **1.6 mm** |
| Inductance | **15.0 mH** |
| Wire length | **83 m** |
| DC Resistance | **6.6 Ω** |
| Current @ 7.5V | **40 mA** |
| Winding direction | **CLOCKWISE** |

### Winding Instructions

```
         ←───────── 18 mm ──────────→
        ┌─────────────────────────────┐
Layer 3 │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ (turns 71-105)
Layer 2 │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ (turns 36-70)
Layer 1 │▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│ (turns 1-35)
        └─────────────────────────────┘
         ╔═══════════════════════════╗
         ║   250 mm diameter form    ║
         ╚═══════════════════════════╝
```

**Materials needed:**
- Plastic cylinder/form: 250 mm diameter
- Copper wire: 0.52 mm (AWG 24), ~90 m length
- Electrical tape for securing layers

**Procedure:**
1. Mark starting point on the 250 mm form
2. Wind **35 turns CLOCKWISE**, close-wound (layer 1)
3. Apply thin tape to secure
4. Wind **35 turns** back over layer 1 (layer 2)
5. Apply tape, then wind **35 turns** for layer 3
6. **Total: 105 turns in 3 layers**

**Final dimensions:** 18 mm long × 1.6 mm thick

### Verification Targets

- [ ] Inductance: 15 mH ± 20\% (12 - 18 mH)
- [ ] DC Resistance: < 10 Ω
- [ ] Current @ 7.5V: 32-48 mA

### Quick Reference - Turns vs Inductance

| Turns | Inductance |
|-------|------------|
| 74 | 7.5 mH |
| 84 | 9.6 mH |
| 95 | 12.2 mH |
| **105** | **15.0 mH** ← Target |
| 116 | 18.1 mH |
| 126 | 21.3 mH |
| 137 | 25.0 mH |

### Resonance Capacitor

| Parameter | Value |
|-----------|-------|
| Capacitance | **470 nF** |
| Type | Film (polyester/polypropylene) |
| Voltage | ≥ 25V |
| Connection | Series with TX coil |
| Resonant freq | ~1900 Hz |

**Purpose:** Converts square wave from MCU into sine wave

```
  MCU Square Wave              LC Tank Output
  ┌───┐   ┌───┐                  ╭───╮   ╭───╮
  │   │   │   │                 ╱     ╲ ╱     ╲
──┘   └───┘   └──      →    ──╱       ╳       ╲──
                               ╲     ╱ ╲     ╱
                                ╰───╯   ╰───╯
```

**Why sine wave matters:**
- Clean phase measurement for DFT
- Accurate ferrous/non-ferrous discrimination  
- Better signal-to-noise ratio
- Lower EMI emissions

---

## RX Coil - PLACEHOLDER (TBD)

### Current Placeholder Values

| Parameter | Value | Status |
|-----------|-------|--------|
| Form diameter | 80 mm | TBD |
| Wire diameter | 0.32 mm (AWG 28) | TBD |
| Target inductance | ≥ 10 mH | Requirement 16 |
| Number of layers | 4 | TBD |
| Winding direction | COUNTER-CLOCKWISE | Fixed |

### Wire Options for RX Coil

The RX coil can use thinner wire since it doesn't carry significant current.

| AWG | Wire ⌀ | Layers | Turns | Height | R_dc |
|-----|--------|--------|-------|--------|------|
| 24 | 0.52 mm | 5 | 180 | 19 mm | 3.7 Ω |
| 26 | 0.40 mm | 4 | 177 | 18 mm | 6.1 Ω |
| 28 | 0.32 mm | 4 | 169 | 14 mm | 9.0 Ω |
| 30 | 0.25 mm | 2 | 188 | 24 mm | 16.3 Ω |

> **Note:** Higher resistance is acceptable for RX coil since it only receives signal.

### RX Resonance Capacitor (TBD)

| Parameter | Value |
|-----------|-------|
| Capacitance | **~680 nF** (depends on final L) |
| Type | Film capacitor |
| Connection | Parallel with RX+Bucking |
| Purpose | Bandpass filter + voltage gain |

Formula: $C = \frac{1}{4\pi^2 f^2 L}$

### To Finalize RX Coil

1. Determine form diameter (depends on physical construction)
2. Choose wire gauge (AWG 26-30 recommended)
3. Update MATLAB script with actual values
4. Run calculator to get exact turns

---

## Bucking Coil - PLACEHOLDER (TBD)

### Purpose

Cancels direct TX field at RX location. Connected in series with RX coil.

### Current Placeholder Values

| Parameter | Value | Status |
|-----------|-------|--------|
| Form diameter | 120 mm | TBD (between TX and RX) |
| Wire diameter | 0.52 mm | TBD |
| Starting turns | 40 | Empirical adjustment needed |
| Number of layers | 2 | TBD |
| Winding direction | COUNTER-CLOCKWISE | Same as RX |
| Adjustment range | 30-50 turns | Empirical |

### Bucking Coil Adjustment Procedure

1. Wind all three coils
2. Connect RX and Bucking in series (same polarity - both counter-clockwise)
3. Power TX with 2 kHz signal (no metal target nearby)
4. Measure RX+Bucking output on oscilloscope
5. Adjust bucking turns:
   - Signal too HIGH → ADD turns
   - Signal INVERTED → REMOVE turns
6. Iterate until signal minimized (< 10 mV)

### To Finalize Bucking Coil

1. Determine form diameter (must fit between TX and RX)
2. Wind with ~40 turns as starting point
3. Leave extra wire for adjustments
4. Fine-tune empirically after assembly

---

## Why Resonance Capacitors?

The LC tank circuit converts the **square wave** from the MCU into a **sine wave**:

1. **MCU outputs square wave** (2 kHz PWM or toggle)
2. **Square wave contains harmonics** (3rd, 5th, 7th... at 6, 10, 14 kHz)
3. **LC tank is tuned to 2 kHz** - only passes fundamental
4. **Result: clean sine wave** in the coil

**Why this matters for metal detection:**
- DFT assumes sinusoidal input
- Clean phase = accurate ferrous/non-ferrous discrimination
- All energy at one frequency = better SNR
- Lower EMI emissions

---

## Summary Table

```
┌─────────────────┬─────────────┬─────────────┬─────────────┐
│ Parameter       │   TX Coil   │   RX Coil   │   Bucking   │
│                 │   (FINAL)   │   (TBD)     │   (TBD)     │
├─────────────────┼─────────────┼─────────────┼─────────────┤
│ Form diameter   │   250 mm    │    TBD      │    TBD      │
│ Wire diameter   │  0.52 mm    │    TBD      │    TBD      │
│ Total turns     │   105       │    TBD      │   ~40       │
│ Layers          │     3       │    TBD      │    TBD      │
│ Axial length    │    18 mm    │   ~20 mm    │    TBD      │
│ Inductance      │   15 mH     │   ≥10 mH    │   ~1 mH     │
│ Resonance C     │   470 nF    │  ~680 nF    │    N/A      │
│ Winding dir.    │  CLOCKWISE  │ COUNTER-CW  │ COUNTER-CW  │
│ Status          │  ✓ READY    │  PENDING    │  PENDING    │
└─────────────────┴─────────────┴─────────────┴─────────────┘
```

---

## Circuit Schematic

```
                         TX CIRCUIT (Series LC)
                         ══════════════════════
                    
        MCU Pin              ┌─────────┐      ┌─────────┐
      (Square Wave)          │    C    │      │  TX L   │
            │                │  470nF  │      │  15mH   │
            ▼                └────┬────┘      └────┬────┘
       ┌─────────┐                │                │
       │ Driver  ├────────────────┴────────────────┴──── GND
       │(H-bridge│
       │ or amp) │           Square wave → Sine wave
       └─────────┘


                         RX CIRCUIT (Parallel LC)
                         ═══════════════════════

            ┌──────────────────────────────────────┐
            │                                      │
            │    ┌─────────┐       ┌─────────┐    │
            ├────┤ RX+Buck │───────┤    C    ├────┤
            │    │ ~11 mH  │       │  680nF  │    │
            │    └─────────┘       └─────────┘    │
            │                                      │
            └──────────────────┬───────────────────┘
                               │
                               ▼
                          To Preamp
                        (Bandpass filtered,
                         voltage amplified)
```

---

## Files

- **Coil_Design_Multilayer.m** - MATLAB calculator (update RX/Bucking parameters when known)
- **coil_functions.m** - Helper functions
- **Coil_Design_Reconciliation.md** - This document

---

## Next Steps

1. ✓ TX Coil - Ready to wind
2. □ Determine RX coil form diameter and wire gauge
3. □ Determine Bucking coil form diameter
4. □ Update MATLAB script with final RX/Bucking values
5. □ Wind all coils
6. □ Verify inductances with LCR meter
7. □ Adjust bucking coil for null

---

#coils #design #TX #RX #bucking #multilayer
