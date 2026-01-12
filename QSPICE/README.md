# QSPICE H-Bridge Simulation

> TX driver simulation for VLF metal detector coil excitation.
> Achieved **~98% efficiency** with optimized RLC filter.

---

## Overview

This folder contains QSPICE simulations for the H-bridge driver that excites the TX coil at 2kHz. The system converts 9V DC battery power to a sinusoidal current through the transmit coil.

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  PWM Input  │────▶│  Dead-Time  │────▶│  Bootstrap  │────▶│   H-Bridge  │────▶ TX Coil
│   (2kHz)    │     │  Generator  │     │ Gate Driver │     │  (IRL530)   │     (LC Tank)
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

---

## Files

| File | Description |
|------|-------------|
| `Hbro_sim.qsch` | Main H-bridge simulation with load |
| `bootstrap.qsch` | Bootstrap gate driver circuit |
| `dead_time.qsch` | Dead-time generator subcircuit |

---

## Hbro_sim.qsch — Main Simulation

### Circuit Topology

```
        +9V (V1, RSER=1.7Ω)
            │
    ┌───────┴───────┐
    │               │
   M6              M3
  (IRL530)       (IRL530)
    │               │
    ├───[L2]───[C1]─┤───[R10]───┐
    │   6.3mH   1µF │    35Ω    │
   M4              M2           │
  (IRL530)       (IRL530)       │
    │               │           │
    └───────┬───────┘           │
           GND◄─────────────────┘
```

### Power Stage

| Component | Part | Specifications |
|-----------|------|----------------|
| M2, M3, M4, M6 | IRL530 | Vds=100V, Ids=15A, Ron=0.16Ω, Qg=17.8nC |
| D3, D4 | 1N4148 | Freewheeling diodes |

### Gate Drivers

| Component | Part | Function |
|-----------|------|----------|
| M1, M7 | BS170 | Level shifter / inverter |
| U1, U2 | INV (OR gate) | Signal inversion |
| D1, D2 | 1N4148 | Bootstrap diode |
| C2, C3 | 1µF | Bootstrap capacitor |
| R3, R7 | Rup (1K) | Pull-up resistor |

### Load (TX Coil Model)

| Component | Value | Purpose |
|-----------|-------|---------|
| L2 | 6.332 mH | TX coil inductance |
| C1 | 1µF (param) | Resonance capacitor |
| R10 | 35Ω | Coil DC resistance |

Resonance frequency:
$$f_0 = \frac{1}{2\pi\sqrt{LC}} = \frac{1}{2\pi\sqrt{6.332\text{mH} \cdot 1\mu\text{F}}} \approx 2\text{kHz}$$

### PWM Signals

| Signal | Definition | Description |
|--------|------------|-------------|
| V3 (PWM1) | `PULSE 0 5 250µ 10n 10n 245µ 500µ` | Phase A, 250µs delay |
| V4 (PWM2) | `PULSE 0 5 0 10n 10n 245µ 500µ` | Phase B, no delay |
| V5 (PWM) | `pulse 5 0 0 10n 10n 0.5/2K 1/2K` | Master 2kHz clock |

The 180° phase shift between PWM1 and PWM2 creates alternating H-bridge states.

### Simulation Commands

```spice
.tran 0 0.2 0.198 1µ          ; Transient analysis, 0.2s
.options savepowers=1          ; Save power data

; Efficiency measurement
.meas Pl rms p(l2)+p(R10)+p(c1)    ; Load power
.meas Psource rms p(v1)            ; Source power
.meas eta param Pl/Psource         ; Efficiency η

; Parameter sweeps (commented)
;.step param R 0.01K 0.91K 0.1K
;.step param c1 0.1µ 1µ 0.1µ
```

---

## bootstrap.qsch — Gate Driver

The bootstrap circuit enables driving high-side N-channel MOSFETs from a ground-referenced PWM signal.

### Operating Principle

```
    +9V
     │
     D1 (BAS521)──────┐
     │                │
    [R1=5Ω]          [C1=1µF]  ← Bootstrap Cap
     │                │
     └────────────────┼──── Vboot (floating supply)
                      │
                     M1 (AO3422, High-side)
                      │
                     OUT ──────────────────── To load
                      │
                     M2 (AO4262E, Low-side)
                      │
                     GND
```

### Bootstrap Sequence

1. **Low-side ON**: M2 conducts, OUT = GND
2. **C1 charges**: Current flows +9V → D1 → R1 → C1 → GND
3. **Low-side OFF**: Dead-time begins
4. **High-side ON**: M1 gate driven by Vboot (OUT + 9V)
5. **C1 discharges**: Provides gate charge for M1

### Components

| Half-Bridge | High-Side | Low-Side | Bootstrap |
|-------------|-----------|----------|-----------|
| Left | M1 (AO3422) | M2 (AO4262E) | D1, C1, R1 |
| Right | M4 (AO3422) | M5 (AO4262E) | D2, C2, R5 |

Gate resistors (R2, R3, R6, R7 = 10Ω) limit di/dt and reduce EMI.

---

## dead_time.qsch — Dead-Time Generator

Prevents shoot-through by ensuring both switches are OFF during transitions.

### Block Diagram

```
                    ┌─────────────┐
     PWM_In ───────▶│   RC Delay  │
                    │  R4=R, C1=C │
                    └──────┬──────┘
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
        ┌──────────┐              ┌──────────┐
        │  OpAmp1  │              │  OpAmp2  │
        │ (comp H) │              │ (comp L) │
        └────┬─────┘              └────┬─────┘
             │                         │
             ▼                         ▼
          H output                  L output
       (high-side)               (low-side)
```

### Timing Network

| Component | Value | Function |
|-----------|-------|----------|
| R1 | 1850Ω | Voltage divider |
| R2 | 650Ω | Voltage divider |
| R3 | 2.5kΩ | Threshold setting |
| R4 | R (param) | Dead-time adjust |
| C1 | C (param) | Dead-time adjust |

Dead-time duration: $t_{dead} \approx R \cdot C$

With R=3kΩ, C=5nF: $t_{dead} \approx 15\mu s$

### Comparators

Two RRopAmp (Rail-to-Rail) configured as comparators:
- **U1**: Generates H output (high-side gate)
- **U2**: Generates L output (low-side gate)

The RC network creates delayed thresholds so:
- When PWM rises: L turns OFF first, then H turns ON (after dead-time)
- When PWM falls: H turns OFF first, then L turns ON (after dead-time)

---

## Simulation Results

### Efficiency

| Parameter | Value |
|-----------|-------|
| Source Power (Psource) | ~500 mW |
| Load Power (Pl) | ~490 mW |
| Efficiency (η) | **~98%** |

### Waveforms

Key signals to plot:
```spice
.plot i(L2)              ; Coil current (should be sinusoidal)
.plot is(M6) is(M3)      ; MOSFET currents
.plot v(G9) v(G3,S3)     ; Gate-source voltages
.plot v(G2) v(G11,S11)   ; Bootstrap gate voltages
```

---

## Design Notes

### Why H-Bridge?

The H-bridge converts DC to AC efficiently by alternately connecting the load to +V and GND. Combined with the LC resonant tank, it produces a clean sinusoidal current at 2kHz.

### Why Bootstrap?

N-channel MOSFETs have lower Rds(on) than P-channel for the same die size. The bootstrap circuit allows using N-channel for both high and low sides, maximizing efficiency.

### Component Selection

- **IRL530**: Logic-level gate (Vgs(th)=2V), suitable for 5V drive
- **BS170**: Small-signal MOSFET for inverter/driver
- **1N4148**: Fast switching diode for bootstrap
- **AO3422/AO4262E**: Alternative MOSFETs in bootstrap.qsch

---

## Running the Simulation

1. Open `Hbro_sim.qsch` in QSPICE
2. Run transient simulation (F5 or green play button)
3. View waveforms: Right-click on nets/components
4. Check efficiency: View `.meas` results in output log

### Parameter Sweeps

Uncomment in `Hbro_sim.qsch` to optimize:
```spice
.step param R 0.01K 0.91K 0.1K    ; Sweep dead-time resistor
.step param c1 0.1µ 1µ 0.1µ       ; Sweep tank capacitor
```

---

*Last updated: January 2026*
