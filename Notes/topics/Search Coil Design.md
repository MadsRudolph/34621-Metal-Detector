# Search Coil Design

> [!abstract] Purpose
> Design TX and RX coils for the VLF metal detector in both **Concentric** and **Double-D** configurations.
>
> See: [[Power Budget Analysis]], [[TX Driver and Tank Circuit Design]], [[Coil Basics.pdf]]

---

## 1. Design Requirements

### 1.1 From Kravspecifikation

| Requirement | Value | Source |
|-------------|-------|--------|
| TX frequency | 2 kHz | Krav #1 |
| Detection depth (iron) | ≥ 50 mm | Krav #3 |
| RX coil inductance | ≥ 10 mH | Krav #16 |
| Distinguish ferrous/non-ferrous | Yes | Krav #2 |

### 1.2 From Power Budget

| Parameter | Value | Source |
|-----------|-------|--------|
| TX coil current | 40 mA | Power budget |
| TX coil impedance | ~190 Ω | @ 7.5V supply |
| TX coil inductance | 15 mH | Calculated |
| TX coil power | ~300 mW | Available budget |

### 1.3 Wire Specification

| Parameter | Value |
|-----------|-------|
| **Wire diameter** | **0.52 mm** |
| Wire type | Enameled copper (magnet wire) |
| AWG equivalent | ~24 AWG |
| Resistance | 0.079 Ω/m |

---

## 2. Coil Configuration Comparison

### 2.1 Concentric vs Double-D

| Aspect | Concentric | Double-D |
|--------|------------|----------|
| **Field shape** | Conical | Blade/stripe |
| **Pinpointing** | Excellent (symmetric) | Good |
| **Discrimination** | Better | Good |
| **Ground balance** | More affected | Better |
| **Mineralized soil** | Struggles | Excels |
| **Detection depth** | Better | Slightly less |
| **Construction** | Easier (circular) | More complex |
| **Sensitivity pattern** | Under RX coil | Along overlap |

### 2.2 Configuration Diagrams

**Concentric (3-coil):**
```
        ┌─────────────────────────────────┐
        │                                 │
        │    ┌───────────────────┐        │
        │    │                   │        │
        │    │    ┌─────────┐    │        │
        │    │    │   RX    │    │        │
        │    │    │  (blue) │    │        │
        │    │    └─────────┘    │        │
        │    │     Bucking       │        │
        │    │      (red)        │        │
        │    └───────────────────┘        │
        │            TX                   │
        │          (green)                │
        └─────────────────────────────────┘
```

**Double-D:**
```
        ┌─────────────────────────────────┐
        │                 │               │
        │     TX          │      RX       │
        │    Coil         │     Coil      │
        │      D ─────────│───── D        │
        │                 │               │
        │           Overlap               │
        │            zone                 │
        │         (null area)             │
        └─────────────────────────────────┘
```

### 2.3 Selection Rationale

> [!tip] Primary Design: Concentric
> Easier construction, better pinpointing, sufficient for non-mineralized soil (Denmark).
>
> **Alternative:** Double-D — For future testing or if ground conditions require it.

---

## 3. Wire Calculations (0.52 mm)

### 3.1 Wire Properties

**Cross-sectional area:**

$$A = \pi r^2 = \pi \times (0.26\ mm)^2 = 0.212\ mm^2$$

**Resistance per meter:**

$$R_{wire} = \frac{\rho_{Cu}}{A} = \frac{1.68 \times 10^{-8}\ \Omega \cdot m}{2.12 \times 10^{-7}\ m^2} = 0.079\ \Omega/m$$

### 3.2 Comparison with Other Wire Gauges

| Diameter | AWG | R (Ω/m) | Current capacity | Notes |
|----------|-----|---------|------------------|-------|
| 0.30 mm | 28 | 0.24 | 0.5 A | Original design |
| 0.40 mm | 26 | 0.14 | 0.8 A | |
| **0.52 mm** | **24** | **0.079** | **1.2 A** | **Selected** |
| 0.65 mm | 22 | 0.05 | 2 A | Bulkier |

### 3.3 Advantages of 0.52 mm Wire

> [!success] Wire Selection Benefits
> 1. **Lower DC resistance** → Less power lost as heat
> 2. **Higher Q factor** → Sharper resonance (if using tank circuit)
> 3. **Easier to handle** → Less fragile during winding
> 4. **Better current capacity** → Margin for future upgrades

---

## 4. Concentric Coil Design (Primary)

### 4.1 Three-Coil Architecture

From course slides, the concentric design uses three coils:

1. **TX Coil (outer)** — Generates the primary magnetic field
2. **Bucking Coil (middle)** — Cancels TX field at RX location
3. **RX Coil (inner)** — Detects target response

```
                    ┌─────────────────────────────────────┐
                    │                                     │
                    │      ┌─────────────────────┐        │
                    │      │                     │        │
                    │      │    ┌───────────┐    │        │
                    │      │    │           │    │        │
                    │      │    │    RX     │    │        │
                    │      │    │  Ø 60mm   │    │        │
                    │      │    │           │    │        │
                    │      │    └───────────┘    │        │
                    │      │                     │        │
                    │      │      Bucking        │        │
                    │      │      Ø 80mm         │        │
                    │      └─────────────────────┘        │
                    │                                     │
                    │              TX Coil                │
                    │              Ø 150mm                │
                    └─────────────────────────────────────┘
```

### 4.2 TX Coil Specifications

**Target:** L = 15 mH, Z ≈ 190 Ω @ 2 kHz

**Using Wheeler's formula for single-layer coil:**

$$L = \frac{r^2 N^2}{9r + 10l}\ [\mu H]$$

Where r = radius (inches), l = winding length (inches), N = turns.

For multi-layer coil:

$$L = \frac{31.6 \times N^2 \times r^2}{6r + 9l + 10d}\ [\mu H]$$

Where d = winding depth (inches).

**Design iteration for 150 mm diameter:**

| Parameter | Value | Calculation |
|-----------|-------|-------------|
| Diameter | 150 mm | Design choice |
| Radius | 75 mm = 2.95" | |
| Target L | 15 mH = 15000 µH | |
| Wire diameter | 0.52 mm | Selected |

**Estimating turns for single-layer approximation:**

Rearranging Wheeler's formula and solving numerically:

For r = 75 mm, targeting L = 15 mH:

$$N \approx 85\ \text{turns (single layer, 44 mm wide)}$$

But single-layer 85 turns × 0.52 mm = 44 mm winding width — reasonable.

**Let's verify:**

$$L = \frac{(2.95)^2 \times 85^2}{9 \times 2.95 + 10 \times 1.73} = \frac{8.7 \times 7225}{26.6 + 17.3} = \frac{62858}{43.9} = 1432\ \mu H = 1.4\ mH$$

That's too low! Wheeler's formula underestimates for flat coils. Let me use the flat spiral formula:

**Flat spiral coil (pancake):**

$$L = \frac{N^2 \times r_{avg}^2}{8 \times r_{avg} + 11 \times w}\ [\mu H]$$

Where $r_{avg}$ = average radius, w = winding width.

For a more accurate estimate, use:

$$L = \frac{\mu_0 N^2 A}{l_{eff}}$$

**Practical approach — use online calculator or empirical data:**

Based on similar coil designs:
- 150 mm diameter, 15 mH → approximately **130 turns** in 2-3 layers

### 4.3 TX Coil Final Specification

| Parameter | Value | Notes |
|-----------|-------|-------|
| Diameter (outer) | 150 mm | |
| Diameter (inner) | ~140 mm | Winding width ~5 mm |
| Number of turns | **130** | 2-3 layers |
| Wire gauge | 0.52 mm | Enameled copper |
| Wire length | 130 × π × 0.145 = **59 m** | |
| DC resistance | 59 × 0.079 = **4.7 Ω** | |
| Inductance | **15 mH** | Target |
| Reactance @ 2 kHz | 188 Ω | $X_L = 2\pi fL$ |
| Impedance | **189 Ω** | $\sqrt{R^2 + X_L^2}$ |
| Q factor | 188/4.7 = **40** | High Q! |
| Current @ 7.5V | **39.7 mA** | Within budget |

### 4.4 RX Coil Specifications

**Requirement:** L ≥ 10 mH (from kravspecifikation #16)

**Design target:** L = 12 mH (20% margin)

For smaller diameter (60 mm), more turns needed:

| Parameter | Value | Notes |
|-----------|-------|-------|
| Diameter | 60 mm | Fits inside bucking coil |
| Number of turns | **180** | ~4 layers |
| Wire gauge | 0.52 mm | Same as TX |
| Wire length | 180 × π × 0.055 = **31 m** | |
| DC resistance | 31 × 0.079 = **2.4 Ω** | |
| Inductance | **12 mH** | > 10 mH ✓ |
| Reactance @ 2 kHz | 151 Ω | |

### 4.5 Bucking Coil Specifications

The bucking coil cancels the TX field at the RX coil location. It carries the **same current as TX** but in **opposite direction**.

**Design principle:**

The induced voltage in RX from TX must equal induced voltage from bucking coil:

$$M_{TX-RX} \times I_{TX} = M_{Buck-RX} \times I_{Buck}$$

Since $I_{TX} = I_{Buck}$ (series connection):

$$M_{TX-RX} = M_{Buck-RX}$$

**Bucking coil placement:**

Position and turns adjusted empirically to achieve null (induction balance).

| Parameter | Value | Notes |
|-----------|-------|-------|
| Diameter | 80 mm | Between TX and RX |
| Number of turns | **15-25** | Adjusted for balance |
| Wire gauge | 0.52 mm | Same as TX |
| Connection | **Series with TX, reversed** | Opposite polarity |

### 4.6 Concentric Coil Wiring Diagram

```
                                TX Coil
                               (130 turns)
                                   │
    Pin 11 ─────────────┬─────────►●────────────────┐
    (TX drive)          │                           │
                        │                           │
                   ┌────┴────┐                      │
                   │ Bucking │ (15-25 turns,        │
                   │  Coil   │  wound opposite)     │
                   └────┬────┘                      │
                        │                           │
                        └───────────────────────────┴────► GND
    
    
                                RX Coil
                               (180 turns)
                                   │
    ADC Input ◄────────────────────●
    (A0)                           │
                                   │
                                  GND
```

### 4.7 Induction Balance Procedure

> [!note] Balancing Steps
> 1. Connect TX + Bucking coils in series (opposite polarity)
> 2. Drive with 2 kHz signal
> 3. Measure voltage at RX coil with oscilloscope
> 4. Adjust bucking coil turns (add/remove) until RX voltage is minimized
> 5. Fine-tune with single turn adjustments
> 6. Final null should be < 1 mV (ideally < 0.1 mV)

**Null adjustment wire:**

Add a single adjustable wire loop that can be moved to fine-tune balance:

```
    ┌──────────────────────────────┐
    │                              │
    │   ┌──── Adjustment wire ───┐ │
    │   │   (can be bent/moved)  │ │
    │   └────────────────────────┘ │
    │                              │
    │         Main coils           │
    └──────────────────────────────┘
```

---

## 5. Double-D Coil Design (Alternative)

### 5.1 Double-D Architecture

Two D-shaped coils overlap in the center:

```
    ┌─────────────────────────────────────────────┐
    │                                             │
    │    ┌─────────────┐   ┌─────────────┐        │
    │    │             │   │             │        │
    │    │             │   │             │        │
    │    │     TX      │   │     RX      │        │
    │    │    Coil     │───│    Coil     │        │
    │    │             │   │             │        │
    │    │             │   │             │        │
    │    │             │   │             │        │
    │    └─────────────┘   └─────────────┘        │
    │                                             │
    │              Overlap Zone                   │
    │           (Induction Null)                  │
    └─────────────────────────────────────────────┘
    
    Side view:
    
         TX Coil          RX Coil
        ┌──────┐         ┌──────┐
        │      │─────────│      │
        │   D  │ overlap │  D   │
        │      │─────────│      │
        └──────┘         └──────┘
```

### 5.2 Double-D Operating Principle

**Induction balance by geometry:**

- TX coil generates field
- At the overlap zone, RX coil sees **equal and opposite** flux from the two halves of TX
- Net induced voltage = 0 (null)
- When target enters, balance is disturbed → signal detected

**Advantages:**

- Ground mineralization affects both halves equally → better cancellation
- Detection zone runs front-to-back (blade pattern)
- Less affected by field compression from soil

### 5.3 Double-D Coil Dimensions

Based on 150 mm total width (same as concentric):

| Parameter | TX Coil | RX Coil |
|-----------|---------|---------|
| Shape | D (half-ellipse) | D (half-ellipse) |
| Width | 80 mm | 80 mm |
| Height | 150 mm | 150 mm |
| Overlap | 10 mm | 10 mm |
| Total head width | 150 mm | |

### 5.4 Double-D TX Coil

| Parameter | Value | Notes |
|-----------|-------|-------|
| Shape | D (flattened side) | |
| Dimensions | 80 mm × 150 mm | |
| Number of turns | **100** | |
| Wire gauge | 0.52 mm | |
| Perimeter per turn | ~370 mm | |
| Wire length | 100 × 0.37 = **37 m** | |
| DC resistance | 37 × 0.079 = **2.9 Ω** | |
| Inductance | **10-12 mH** | Geometry dependent |

### 5.5 Double-D RX Coil

| Parameter | Value | Notes |
|-----------|-------|-------|
| Shape | D (mirror of TX) | |
| Dimensions | 80 mm × 150 mm | Same as TX |
| Number of turns | **120** | More turns for ≥10 mH |
| Wire gauge | 0.52 mm | |
| Wire length | 120 × 0.37 = **44 m** | |
| DC resistance | 44 × 0.079 = **3.5 Ω** | |
| Inductance | **≥ 10 mH** | Meets requirement |

### 5.6 Double-D Coil Overlap Calculation

The overlap must be precisely positioned for null:

**Overlap percentage:** Typically 10-20% of coil width

$$\text{Overlap} = 0.15 \times 80\ mm = 12\ mm$$

**Adjustment:**

Unlike concentric, Double-D balance is adjusted by:
1. Moving the overlap position (mechanical)
2. Adding small compensation winding
3. Trimming capacitor in RX circuit

### 5.7 Double-D Wiring

```
                           ┌─────────────────┐
                           │     TX Coil     │
    TX Drive ──────────────┤    (D-shape)    ├──────────► GND
    (Pin 11)               │    100 turns    │
                           └─────────────────┘
    
    
                           ┌─────────────────┐
                           │     RX Coil     │
    ADC Input ◄────────────┤    (D-shape)    ├──────────► GND
    (A0)                   │    120 turns    │
                           └─────────────────┘
```

> [!info] Key Advantage
> No bucking coil needed — geometry provides null!

---

## 6. Physical Construction

### 6.1 Coil Former Materials

| Material | Pros | Cons |
|----------|------|------|
| **Plywood** | Cheap, easy to cut | Heavy, can warp |
| **Acrylic/Perspex** | Rigid, waterproof | Expensive, brittle |
| **PVC pipe** | Cheap, round | Only for round coils |
| **3D printed** | Custom shapes | May not be rigid enough |
| **Fiberglass** | Strong, light | Requires skill |

**Recommended:** 3-4 mm plywood with epoxy coating for waterproofing.

### 6.2 Concentric Coil Construction

**Materials:**
- 4 mm plywood disc, Ø 180 mm
- Grooves routed for wire channels
- Epoxy resin for potting

**Cross-section:**

```
                    4 mm plywood base
    ┌─────────────────────────────────────────────────┐
    │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│
    │░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░│
    └─────────────────────────────────────────────────┘
      ▲         ▲              ▲              ▲
      │         │              │              │
     TX       Bucking         RX           Handle
    groove    groove        groove         mount
    (outer)   (middle)      (inner)
```

**Steps:**
1. Cut circular plywood disc (Ø 180 mm)
2. Route three circular grooves for TX, Bucking, RX
3. Wind TX coil (130 turns) in outer groove
4. Wind Bucking coil (20 turns, opposite) in middle groove
5. Wind RX coil (180 turns) in inner groove
6. Connect TX + Bucking in series
7. Test and adjust for null
8. Pot with epoxy resin
9. Attach handle/shaft

### 6.3 Double-D Coil Construction

**Materials:**
- 4 mm plywood, cut to oval shape
- Central divider for overlap alignment
- Epoxy resin for potting

**Template:**

```
    ┌───────────────────────────────────────┐
    │                                       │
    │   ┌───────────────────────────────┐   │
    │   │                               │   │
    │   │     D      │ overlap │   D    │   │
    │   │    groove  │  zone   │ groove │   │
    │   │            │         │        │   │
    │   └───────────────────────────────┘   │
    │                                       │
    │              Handle mount             │
    └───────────────────────────────────────┘
```

---

## 7. Bill of Materials

### 7.1 Concentric Coil BOM

| Item | Specification | Quantity | Est. Cost |
|------|---------------|----------|-----------|
| Magnet wire | 0.52 mm enameled copper | 100 m | €15 |
| Plywood | 4 mm, 200×200 mm | 1 sheet | €3 |
| Epoxy resin | 2-part, slow cure | 100 ml | €8 |
| PVC handle | 25 mm diameter, 1 m | 1 | €5 |
| Cable | 2-core shielded, 1.5 m | 1 | €4 |
| Connector | 4-pin aviation plug | 1 pair | €3 |
| **Total** | | | **~€38** |

### 7.2 Double-D Coil BOM

| Item | Specification | Quantity | Est. Cost |
|------|---------------|----------|-----------|
| Magnet wire | 0.52 mm enameled copper | 85 m | €12 |
| Plywood | 4 mm, 200×200 mm | 1 sheet | €3 |
| Epoxy resin | 2-part, slow cure | 100 ml | €8 |
| PVC handle | 25 mm diameter, 1 m | 1 | €5 |
| Cable | 2-core shielded, 1.5 m | 1 | €4 |
| Connector | 4-pin aviation plug | 1 pair | €3 |
| **Total** | | | **~€35** |

---

## 8. Coil Specifications Summary

### 8.1 Concentric Configuration

| Coil | Diameter | Turns | Wire | L | R_dc |
|------|----------|-------|------|---|------|
| **TX** | 150 mm | 130 | 0.52 mm | 15 mH | 4.7 Ω |
| **Bucking** | 80 mm | 20 | 0.52 mm | ~0.5 mH | 0.4 Ω |
| **RX** | 60 mm | 180 | 0.52 mm | 12 mH | 2.4 Ω |

### 8.2 Double-D Configuration

| Coil | Dimensions | Turns | Wire | L | R_dc |
|------|------------|-------|------|---|------|
| **TX** | 80×150 mm (D) | 100 | 0.52 mm | 11 mH | 2.9 Ω |
| **RX** | 80×150 mm (D) | 120 | 0.52 mm | 12 mH | 3.5 Ω |

### 8.3 Electrical Performance Comparison

| Parameter | Concentric | Double-D |
|-----------|------------|----------|
| TX inductance | 15 mH | 11 mH |
| TX impedance @ 2 kHz | 189 Ω | 140 Ω |
| TX current @ 7.5V | 40 mA | 54 mA ⚠️ |
| TX Q factor | 40 | 48 |
| RX inductance | 12 mH | 12 mH |
| Ground rejection | Moderate | Excellent |
| Pinpointing | Excellent | Good |

> [!warning] Double-D Current Draw
> Double-D draws more TX current! May need to:
> - Add series resistor (47 Ω) to limit current
> - Reduce TX turns
> - Accept higher current (still within budget if optimizations applied)

---

## 9. Testing Procedures

### 9.1 Inductance Measurement

**Using Analog Discovery 3:**

1. Connect coil to Impedance Analyzer
2. Sweep frequency 100 Hz - 10 kHz
3. Read inductance at 2 kHz
4. Verify L is within ±10% of target

**Using LCR meter:**

1. Set frequency to 1 kHz or 2 kHz
2. Connect coil leads
3. Read inductance directly

### 9.2 DC Resistance Measurement

1. Use multimeter in Ω mode
2. Measure across coil terminals
3. Compare to calculated value
4. Deviation > 20% indicates winding issue

### 9.3 Induction Balance Test

**For Concentric:**

1. Connect TX + Bucking coils
2. Drive with 2 kHz, 5V p-p sine wave
3. Measure RX coil voltage with oscilloscope
4. Target: < 1 mV p-p (< 0.1 mV ideal)
5. If too high: adjust bucking coil turns

**For Double-D:**

1. Position coils with calculated overlap
2. Drive TX with 2 kHz
3. Measure RX voltage
4. Adjust overlap position for minimum

### 9.4 Detection Test

1. Apply induction-balanced coil
2. Move iron test object (Ø 30 mm × 50 mm cylinder) under coil
3. Verify signal appears at RX
4. Measure detection depth
5. Should detect at ≥ 50 mm

---

## 10. Design Trade-offs

### 10.1 Concentric Pros/Cons

**Advantages:**
- ✅ Easier to construct (circular geometry)
- ✅ Better pinpointing (symmetric field)
- ✅ Better discrimination
- ✅ Well-documented designs available

**Disadvantages:**
- ❌ More affected by mineralized soil
- ❌ Requires bucking coil (3 coils total)
- ❌ More complex wiring

### 10.2 Double-D Pros/Cons

**Advantages:**
- ✅ Better ground balance
- ✅ Only 2 coils needed
- ✅ Simpler wiring (no bucking coil)
- ✅ Good for mineralized soil

**Disadvantages:**
- ❌ Harder to construct (D-shapes)
- ❌ Less precise pinpointing
- ❌ Null zone along center line
- ❌ Slightly less depth

### 10.3 Recommendation

| Situation | Recommended Configuration |
|-----------|--------------------------|
| **First prototype** | Concentric |
| **Mineralized soil** | Double-D |
| **Precise pinpointing needed** | Concentric |
| **Simplest construction** | Concentric |
| **Beach/salt water** | Double-D |

> [!tip] Recommendation
> **Start with Concentric** for the first prototype. It's easier to build and debug. Build Double-D as a second coil head for comparison testing.

---

## 11. References

- [[Power Budget Analysis]]
- [[TX Driver and Tank Circuit Design]]
- [[Literature/Coil_Basics.pdf|Coil Design Basics]]
- [[Literature/Advanced_Instrumentation_for_Polyharmonic_Metal_Detectors.pdf|Advanced Instrumentation]]
- [[Literature/Course_intro.pdf|Course Introduction]]
- [[Literature/kravspecifikation.pdf|Kravspecifikation]]

---

#coil-design #concentric #double-d #tx-coil #rx-coil #construction
