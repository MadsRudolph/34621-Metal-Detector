# 🔄 Coil Configuration: Concentric vs Double-D

> Comparison of coil configurations for VLF metal detector design.
> 
> 📚 **Theory:** [Coil_Basics.pdf](obsidian://open?vault=Obsidian&file=..%2FLiterature%2FCoil_Basics) | [Advanced_Instrumentation](obsidian://open?vault=Obsidian&file=..%2FLiterature%2FAdvanced_Instrumentation_for_Polyharmonic_Metal_Detectors)

---

## Quick Comparison

| Aspect | Concentric | Double-D (DD) |
|--------|------------|---------------|
| **Construction** | Easier (circular coils) | Harder (D-shaped, precise overlap) |
| **Sensitivity** | ✅ Better (non-mineralized soil) | Slightly less |
| **Ground Rejection** | ❌ Poor in mineralized soil | ✅ Excellent |
| **Pinpointing** | ✅ Best (center-focused) | Good (blade pattern) |
| **Discrimination** | ✅ Better | Good |
| **Coverage per Sweep** | Cone pattern | ✅ Wider blade pattern |
| **Depth** | Good | ✅ Better in bad ground |
| **Detection Pattern** | Cone (circular) | Blade (line) |

---

## Detection Field Patterns

### Concentric
```
        Top View              Side View (Cross-section)
      ┌─────────┐                    ╱╲
     ╱    RX    ╲                   ╱  ╲
    │   ┌───┐    │                 ╱    ╲
    │   │ ● │    │  TX            ╱ CONE ╲
    │   └───┘    │               ╱        ╲
     ╲          ╱               ▼──────────▼
      └─────────┘              Best detection at center
```
- **Pattern:** Cone-shaped, strongest at center
- **Pinpointing:** Excellent — target is directly below center
- **Sweep:** Requires more overlap between sweeps

### Double-D
```
        Top View              Side View (Cross-section)
      ┌─────────┐                    
     ╱ D     D  ╲                 │        │
    │   ╲   ╱    │               ╱│        │╲
    │    ╲ ╱     │              ╱ │ BLADE  │ ╲
    │    ╱ ╲     │             ╱  │        │  ╲
     ╲  ╱   ╲   ╱             ▼───┴────────┴───▼
      └─────────┘              Detection along center line
         ▲
    Overlap zone
   (detection area)
```
- **Pattern:** Blade-shaped along the overlap line
- **Pinpointing:** Good — target is below the center line
- **Sweep:** Better ground coverage per pass

---

## Hardware Comparison

### Concentric Configuration

From your course materials (slide 37):
```
┌──────────────────────────────────────┐
│            TX Coil (outer)           │  Green - Transmit
│   ┌────────────────────────────┐     │
│   │      Feedback/Bucking      │     │  Red - Bucking
│   │   ┌────────────────────┐   │     │
│   │   │     RX Coil        │   │     │  Blue - Receive
│   │   │                    │   │     │
│   │   └────────────────────┘   │     │
│   └────────────────────────────┘     │
└──────────────────────────────────────┘
```

**Components needed:**
| Component | Purpose | Notes |
|-----------|---------|-------|
| TX Coil | Transmit 2 kHz field | Outer, largest diameter |
| Bucking/Feedback Coil | Cancel TX coupling to RX | Wired in opposition to TX |
| RX Coil | Receive reflected signal | Inner, L ≥ 10 mH per kravspec |

**Balancing:** The feedback coil cancels the direct TX→RX coupling so only the metal reflection is detected.

### Double-D Configuration

```
┌─────────────────────────────────────┐
│                                     │
│    ┌─────────┐  ┌─────────┐        │
│    │         │  │         │        │
│    │   TX    │╲╱│   RX    │        │
│    │    D    │╱╲│    D    │        │
│    │         │  │         │        │
│    └─────────┘  └─────────┘        │
│                                     │
└─────────────────────────────────────┘
        ▲           ▲
        └─────┬─────┘
         Overlap zone
      (induction balanced)
```

**Components needed:**
| Component | Purpose | Notes |
|-----------|---------|-------|
| TX Coil (D-shaped) | Transmit field | One half |
| RX Coil (D-shaped) | Receive signal | Other half, overlaps TX |

**Balancing:** The overlap geometry naturally creates a null zone where TX coupling cancels.

---

## Electrical Parameters

### Typical Values (from ATMID detector in literature)

| Parameter | Concentric (estimated) | Double-D (measured) |
|-----------|------------------------|---------------------|
| TX Resistance | ~1-2 Ω | 1.2 Ω |
| TX Inductance | ~0.5-1 mH | 0.774 mH |
| RX Resistance | ~100-200 Ω | 182 Ω |
| RX Inductance | ≥10 mH (per kravspec) | 3.35 mH |
| Mutual Inductance | Minimized by bucking | 0.1 µH |

### Equivalent Circuit (Both types)

```
TX Side                    RX Side
   │                          │
  ┌┴┐ R_tx                   ┌┴┐ R_rx
  └┬┘                        └┬┘
   │                          │
  ┌┴┐                        ┌┴┐
  │ │ L_tx                   │ │ L_rx
  └┬┘                        └┬┘
   │                          │
   └──────── M (mutual) ──────┘
```

---

## Software/DSP Differences

### What Stays the Same ✅

| Component | Notes |
|-----------|-------|
| DFT calculation | Same single-bin at 2 kHz |
| IIR filtering | Same smoothing algorithm |
| Phase detection | Same ferrous/non-ferrous threshold (~65°) |
| Sampling rate | Same 8 kHz |
| Timer configuration | Same hardware PWM |

### What May Need Adjustment ⚠️

| Parameter | Concentric | Double-D | Code Impact |
|-----------|------------|----------|-------------|
| Signal amplitude | Higher | Lower | Adjust `DFT_NORMALIZATION_FACTOR` |
| Calibration values | Different | Different | Re-run calibration routine |
| Detection threshold | May differ | May differ | Adjust `DETECTION_THRESHOLD_PERCENT` |
| Phase offset | May differ | May differ | Adjust calibration offset |
| Amplifier gain | May need less | May need more | Hardware change |

### Code Changes Required

```c
// hw_config.h - May need different values for each coil type

#ifdef COIL_CONCENTRIC
    #define DFT_NORMALIZATION_FACTOR  (100.0f / (16 * 4095 * 141.0f))
    #define DETECTION_THRESHOLD_PERCENT 0.2f
    #define AMPLIFIER_GAIN 10  // Lower gain needed
#endif

#ifdef COIL_DOUBLE_D
    #define DFT_NORMALIZATION_FACTOR  (100.0f / (16 * 4095 * 100.0f))  // Adjusted
    #define DETECTION_THRESHOLD_PERCENT 0.15f  // More sensitive threshold
    #define AMPLIFIER_GAIN 20  // Higher gain needed
#endif
```

**Bottom line:** The coils are largely interchangeable in software — only calibration constants change.

---

## Construction Complexity

### Concentric

**Difficulty:** ⭐⭐ Medium

1. Wind circular TX coil (easy)
2. Wind circular bucking coil (easy)
3. Wind circular RX coil (easy)
4. **Critical:** Position bucking coil to achieve null balance
5. Fine-tune with single wire loop
6. Pot in epoxy

**Challenges:**
- Getting exact bucking coil position
- Maintaining balance after potting

### Double-D

**Difficulty:** ⭐⭐⭐ Hard

1. Create D-shaped forms (need 3D print or laser cut)
2. Wind D-shaped TX coil (harder to wind evenly)
3. Wind D-shaped RX coil (harder to wind evenly)
4. **Critical:** Precise overlap positioning for null
5. Pot in epoxy

**Challenges:**
- D-shaped winding is tricky
- Overlap geometry must be precise
- Mechanical tolerances are strict

---

## Ground Balancing Performance

### Why DD is Better in Mineralized Soil

```
Concentric in mineralized soil:
┌────────────────────────────┐
│  TX field gets compressed  │
│  unevenly by soil minerals │
│                            │
│    ╱──────╲   Squashed     │
│   ╱   RX   ╲  field        │
│  │  ┌───┐   │ creates      │
│   ╲ └───┘  ╱  imbalance    │
│    ╲──────╱                │
└────────────────────────────┘
Result: False signals from ground

Double-D in mineralized soil:
┌────────────────────────────┐
│  TX compression affects    │
│  both halves equally       │
│                            │
│   ┌───┐╲╱┌───┐            │
│   │ T │╱╲│ R │ Balance     │
│   └───┘  └───┘ maintained  │
│                            │
└────────────────────────────┘
Result: Ground effect cancels out
```

---

## Recommendation for Your Project

### Consider Concentric If:
- ✅ First time building coils
- ✅ Testing in lab environment (low mineralization)
- ✅ Want easier construction
- ✅ Need best pinpointing accuracy
- ✅ Time is limited

### Consider Double-D If:
- ✅ Have access to 3D printer for D-forms
- ✅ Want best real-world performance
- ✅ Planning to test outdoors in Denmark (moderate mineralization)
- ✅ Team has some coil-winding experience
- ✅ Want the learning experience

### Hybrid Approach (Recommended)

1. **Start with Concentric** — get the electronics and DSP working
2. **Build DD later** — as an upgrade if time permits
3. **Compare both** — great content for the report!

The same PCB and code work for both — just swap coils and recalibrate.

---

## Parts List Comparison

### Concentric

| Part | Quantity | Notes |
|------|----------|-------|
| Magnet wire (0.3-0.5mm) | ~50m | For all three coils |
| Circular form (large) | 1 | TX coil, ~150-200mm diameter |
| Circular form (medium) | 1 | Bucking coil |
| Circular form (small) | 1 | RX coil, for L ≥ 10mH |
| Epoxy/potting compound | ~200ml | |
| Fine wire for tuning | 1m | Single strand for balance trim |

### Double-D

| Part | Quantity | Notes |
|------|----------|-------|
| Magnet wire (0.3-0.5mm) | ~40m | For both D coils |
| D-shaped form | 2 | 3D printed, ~150-200mm |
| Overlap jig | 1 | To hold D's in position |
| Epoxy/potting compound | ~200ml | |

---

## References

- [[Coil Basics.pdf|Coil Basics]] — Detailed coil theory
- [[Advanced Instrumentation for Polyharmonic Metal Detectors.pdf|Advanced Instrumentation]] — DD coil measurements
- [[Course_intro.pdf|Course Introduction]] — Concentric configuration (slide 37)
- [[kravspecifikation.pdf|Kravspecifikation]] — RX coil ≥ 10 mH requirement

---

#coils #hardware #design-decision
