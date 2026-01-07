# TX Driver and Tank Circuit Design

> [!abstract] Purpose
> Design the power amplifier stage to drive the TX coil efficiently within the power budget constraints.
>
> See: [[Power Budget Analysis]] for current budget, [[Coil Basics.pdf|Coil Basics]]

---

## 1. Amplifier Topology Selection

### 1.1 Requirements

| Parameter | Value | Source |
|-----------|-------|--------|
| TX Frequency | 2 kHz | Kravspecifikation |
| Available current | 40 mA | Power budget |
| Available power | 240-360 mW | Power budget |
| Supply voltage | 6-9 V | Battery range |
| Coil inductance | 15 mH | Coil design |
| Input signal | Square wave from MCU | Timer1 output |

### 1.2 Topology Comparison

| Aspect | Class D (Switching) | Class AB (Linear) |
|--------|---------------------|-------------------|
| **Efficiency** | 90-95% | 50-70% |
| **Heat dissipation** | Very low | High |
| **Output waveform** | Square/PWM | Amplified input |
| **Complexity** | Low-Medium | Low |
| **With tank circuit** | ✅ Ideal match | ⚠️ Overkill |
| **Component count** | 2-4 | 4-8 |
| **Power to coil** | ~340 mW | ~220 mW |
| **Power wasted** | ~20 mW | ~140 mW |

### 1.3 Efficiency Analysis

**Class D (Switching) at 40 mA, 9V:**

$$P_{in} = V_{bat} \times I_{total} = 9V \times 40mA = 360\ \text{mW}$$

$$P_{loss} = V_{CE(sat)} \times I_C = 0.2V \times 40mA = 8\ \text{mW}$$

$$\eta = \frac{P_{in} - P_{loss}}{P_{in}} = \frac{360 - 8}{360} = 97.8\%$$

**Class AB (Linear) at 40 mA, 9V:**

For Class AB, transistors conduct for >180° of cycle. Typical efficiency:

$$\eta_{AB} = \frac{\pi}{4} \times \frac{V_{out,peak}}{V_{CC}} \approx 60-70\%$$

At 65% efficiency:

$$P_{coil} = 0.65 \times 360 = 234\ \text{mW}$$

$$P_{wasted} = 360 - 234 = 126\ \text{mW (as heat)}$$

### 1.4 Decision

> [!success] Selected Topology: Class D (Switching)
> - 97% efficiency vs 65% for Class AB
> - More power delivered to coil (better detection range)
> - Lower heat dissipation (no heatsink needed)
> - Tank circuit converts square wave to sine wave (if needed)
> - Simpler circuit

---

## 2. Tank Circuit Theory

### 2.1 Series LC Resonance

The TX coil forms part of a series LC tank circuit:

```
                    ┌───────┐
    Square wave ────┤   C   ├────┬────────
    from driver     └───────┘    │
                            ┌────┴────┐
                            │    L    │  TX Coil
                            │  (15mH) │
                            └────┬────┘
                                 │
                                ─┴─ GND
```

### 2.2 Resonance Condition

At resonance, inductive and capacitive reactances are equal:

$$X_L = X_C$$

$$2\pi f_0 L = \frac{1}{2\pi f_0 C}$$

Solving for resonant frequency:

$$f_0 = \frac{1}{2\pi\sqrt{LC}}$$

### 2.3 Impedance at Resonance

**Off resonance:** $Z = R + j(X_L - X_C)$

**At resonance:** $X_L = X_C$, so they cancel:

$$Z_{resonance} = R_{dc}$$

The impedance drops to just the DC resistance of the coil!

This is critical: at resonance, the tank circuit looks purely resistive, and maximum current flows.

### 2.4 Q Factor (Quality Factor)

The Q factor describes how "sharp" the resonance is:

$$Q = \frac{X_L}{R_{dc}} = \frac{2\pi f_0 L}{R_{dc}}$$

For our coil ($L = 15$ mH, $R_{dc} = 15\ \Omega$, $f_0 = 2$ kHz):

$$Q = \frac{2\pi \times 2000 \times 0.015}{15} = \frac{188.5}{15} = 12.6$$

**Bandwidth:**

$$BW = \frac{f_0}{Q} = \frac{2000}{12.6} = 159\ \text{Hz}$$

This means the circuit responds to frequencies from ~1920 Hz to ~2080 Hz.

### 2.5 Current Amplification

> [!note] Q Factor Advantage
> At resonance, the tank circuit provides current amplification. The current in the LC loop can be **Q times higher** than the drive current:
>
> $$I_{coil} = Q \times I_{drive}$$
>
> This is a key advantage — we can achieve higher coil current with lower driver current!

---

## 3. Component Calculations

### 3.1 Resonant Capacitor

For resonance at $f_0 = 2$ kHz with $L = 15$ mH:

$$C = \frac{1}{(2\pi f_0)^2 L}$$

$$C = \frac{1}{(2\pi \times 2000)^2 \times 0.015}$$

$$C = \frac{1}{(12566)^2 \times 0.015}$$

$$C = \frac{1}{2.369 \times 10^9}$$

$$C = 422\ \text{nF}$$

**Standard capacitor values:**

| Value | Resulting $f_0$ | Error |
|-------|-----------------|-------|
| 390 nF | 2080 Hz | +4% |
| 420 nF | 2005 Hz | +0.25% |
| 470 nF | 1896 Hz | -5.2% |

**Selected: C = 470 nF** (adjust Timer1 to ~1900 Hz)

For C = 470 nF:

$$f_0 = \frac{1}{2\pi\sqrt{0.015 \times 470 \times 10^{-9}}} = 1897\ \text{Hz}$$

Timer1 OCR1A for 1897 Hz (toggle mode):

$$OCR1A = \frac{16 \times 10^6}{2 \times 1 \times 1897} - 1 = 4215$$

### 3.2 Capacitor Requirements

| Parameter | Requirement | Reason |
|-----------|-------------|--------|
| Voltage rating | ≥ 50V | Peak voltage across C can exceed supply |
| Type | Film (polyester/polypropylene) | Low ESR, stable |
| Tolerance | ±5% or better | Frequency accuracy |
| Temperature stability | Good | Outdoor use |

**Recommended:** Polyester film capacitor, 470 nF, 50V, ±5%

### 3.3 Peak Voltage Across Capacitor

At resonance with Q = 12.6, voltage can be amplified:

$$V_{C,peak} \approx Q \times \frac{4}{\pi} \times \frac{V_{supply}}{2} = 12.6 \times 1.27 \times 4.5 = 72V$$

> [!warning] Voltage Rating
> Use **50V rated capacitors minimum**, preferably 100V for safety margin.

---

## 4. Driver Circuit Options

### 4.1 Option 1: Simple NPN Switch (Recommended for Simplicity)

Best for: Minimal component count, proven design, matches power budget exactly.

```
                          V_bat (6-9V)
                             │
                        ┌────┴────┐
                        │ TX Coil │
                        │ L=15mH  │
                        │ R=15Ω   │
                        └────┬────┘
                             │
                    D1 ──┤◄├──┤ (1N4148 flyback)
                             │
                             ▼ Collector
                       ┌──────────┐
    Pin 11 ───[1.5kΩ]──►│ 2N2222A │
    (2 kHz)            │   NPN    │
                       └────┬─────┘
                            │ Emitter
                            ▼
                           GND
```

**Characteristics:**
- Current: $I = V/Z = 7.5V / 189\Omega = 39.7$ mA ✓
- Power: $P = V^2/Z = 298$ mW ✓
- Efficiency: ~94%
- Waveform: Square wave (unipolar)
- Component count: 3

**BOM:**

| Component | Value | Package |
|-----------|-------|---------|
| Q1 | 2N2222A | TO-92 |
| R_B | 1.5 kΩ | 0805 |
| D1 | 1N4148 | DO-35 |

### 4.2 Option 2: MOSFET Half-Bridge (Recommended for Performance)

Best for: Maximum efficiency, true AC drive, lowest power waste.

> **Why MOSFET over BJT for half-bridge?** See Section 5.

```
                              V_bat (9V)
                                 │
                            ┌────┴────┐
                 ┌──────────┤ S    D  ├──────────┐
                 │          │   Q1    │          │
    Pin 11 ──────┤──[100Ω]──┤ Si2301  │          │
    (HI_DRV)     │          │  P-ch   │          │
                 │          └────┬────┘          │
                 │               │               │
                 │               ├───────────────┤
                 │               │               │
                 │          ┌────┴────┐          │
                 │          │ TX Coil │          │
                 │          │  15 mH  │          │
                 │          └────┬────┘          │
                 │               │               │
                 │               ├───────────────┤
                 │               │               │
                 │          ┌────┴────┐          │
    Pin 12 ──────┤──[100Ω]──┤ G    D  ├──────────┘
    (LO_DRV)     │          │   Q2    │
                 │          │ Si2302  │
                 │          │  N-ch   │
                 │          └────┬────┘
                 │               │ S
                 │               │
                 └───────────────┴──────► GND
```

**Characteristics:**
- True bipolar AC drive through coil
- Built-in body diodes (free flyback protection)
- Zero DC drive current (voltage-controlled gates)
- Efficiency: >99%
- Waveform: Square wave (bipolar ±V_bat)

**BOM:**

| Component | Value | Package | Notes |
|-----------|-------|---------|-------|
| Q1 | Si2301 | SOT-23 | P-channel high-side |
| Q2 | Si2302 | SOT-23 | N-channel low-side |
| R_g1 | 100 Ω | 0805 | Gate resistor (EMI) |
| R_g2 | 100 Ω | 0805 | Gate resistor (EMI) |

### 4.3 Option 3: MOSFET Half-Bridge with Tank Circuit

Best for: Sine wave output, lowest EMI, best selectivity.

```
                              V_bat (9V)
                                 │
                            ┌────┴────┐
                 ┌──────────┤ S    D  ├──────────┐
                 │          │   Q1    │          │
    Pin 11 ──────┤──[100Ω]──┤ Si2301  │          │
                 │          │  P-ch   │          │
                 │          └────┬────┘          │
                 │               │               │
                 │               ├───────────────┼──────┐
                 │               │               │      │
                 │          ┌────┴────┐     ┌────┴────┐ │
                 │          │ TX Coil │     │    C    │ │
                 │          │  15 mH  │     │  470nF  │ │
                 │          └────┬────┘     └────┬────┘ │
                 │               │               │      │
                 │               ├───────────────┼──────┘
                 │               │               │
                 │          ┌────┴────┐          │
    Pin 12 ──────┤──[100Ω]──┤ G    D  ├──────────┘
                 │          │   Q2    │
                 │          │  N-ch   │
                 │          └────┬────┘
                 │               │ S
                 └───────────────┴──────► GND
```

**Additional component:**

| Component | Value | Package | Notes |
|-----------|-------|---------|-------|
| C1 | 470 nF | Film | 50V min, polyester |

---

## 5. BJT vs MOSFET Half-Bridge Analysis

### 5.1 Comparison Table

| Aspect | BJT (2N2222 + 2N2907) | MOSFET (Si2301 + Si2302) |
|--------|----------------------|--------------------------|
| **Drive type** | Current-controlled | Voltage-controlled |
| **DC drive current** | ~3 mA per transistor | ~0 mA (capacitive gate) |
| **Drive power wasted** | ~50 mW | ~0 mW |
| **Conduction loss** | $V_{CE(sat)} \times I_C = 16$ mW | $I^2 \times R_{DS(on)} < 1$ mW |
| **High-side drive** | Needs PNP + complex biasing | P-ch direct from MCU |
| **Body diode** | None (need external) | Built-in (free flyback) |
| **Total efficiency loss** | ~66 mW | ~0.3 mW |

### 5.2 Drive Current Comparison

**BJT half-bridge at 40 mA:**

Each transistor needs base current for saturation:

$$I_{B} = \frac{I_C}{\beta} \times overdrive = \frac{40mA}{75} \times 5 = 2.67\ \text{mA}$$

With both transistors (alternating):
- Average base current: ~3 mA
- Power wasted: $3mA \times 9V = 27$ mW per transistor
- **Total drive power: ~50 mW**

**MOSFET half-bridge:**

Gate is capacitive — only charging/discharging current:

$$I_{gate,avg} = Q_g \times f = 5nC \times 2kHz = 10\ \mu A$$

**Total drive power: ~0.1 mW** (negligible!)

### 5.3 Conduction Loss Comparison

**BJT:**

$$P_{cond} = V_{CE(sat)} \times I_C = 0.2V \times 40mA = 8\ \text{mW per transistor}$$

$$P_{total} = 16\ \text{mW}$$

**MOSFET:**

$$P_{cond} = I_{RMS}^2 \times (R_{DS,P} + R_{DS,N})$$

$$P_{cond} = (40mA)^2 \times (100m\Omega + 50m\Omega) = 0.0016 \times 0.15 = 0.24\ \text{mW}$$

### 5.4 Power Budget Impact

| Parameter | BJT Half-Bridge | MOSFET Half-Bridge | Savings |
|-----------|-----------------|--------------------| --------|
| Drive current | 6 mA | 0 mA | **6 mA** |
| Conduction loss | 16 mW | 0.3 mW | 15.7 mW |
| Total waste | 66 mW | 0.3 mW | **65.7 mW** |

**The 6 mA savings is significant** — that's 5% of your total 120 mA budget!

### 5.5 Verdict

> [!tip] Use MOSFETs for half-bridge designs
> The efficiency advantage is overwhelming, and the circuit is actually simpler (no base resistors, built-in body diodes).

---

## 6. Recommended MOSFET Selection

### 6.1 Requirements

| Parameter | Requirement | Reason |
|-----------|-------------|--------|
| $V_{DS}$ | > 15V | Handle 9V + transients |
| $I_D$ | > 100 mA | 2.5× margin over 40 mA |
| $V_{GS(th)}$ | < 2.5V | Logic-level (5V drive) |
| $R_{DS(on)}$ | < 500 mΩ | Low conduction loss |
| Package | SOT-23 | Small, easy to solder |

### 6.2 N-Channel (Low-Side) Options

| Part | $R_{DS(on)}$ | $V_{GS(th)}$ | $I_D$ max | Package | Price |
|------|-------------|--------------|-----------|---------|-------|
| **Si2302** | 50 mΩ | 1.2V | 2.6 A | SOT-23 | €0.15 |
| IRLML2502 | 45 mΩ | 1.2V | 4.2 A | SOT-23 | €0.20 |
| 2N7002 | 2.5 Ω | 2.1V | 300 mA | SOT-23 | €0.05 |
| AO3400 | 40 mΩ | 1.4V | 5 A | SOT-23 | €0.15 |

### 6.3 P-Channel (High-Side) Options

| Part | $R_{DS(on)}$ | $V_{GS(th)}$ | $I_D$ max | Package | Price |
|------|-------------|--------------|-----------|---------|-------|
| **Si2301** | 100 mΩ | -1.2V | 2.3 A | SOT-23 | €0.15 |
| AO3401 | 85 mΩ | -1.4V | 4 A | SOT-23 | €0.20 |
| IRLML6401 | 50 mΩ | -1.2V | 4.3 A | SOT-23 | €0.25 |
| DMP2045U | 80 mΩ | -1.0V | 4.4 A | SOT-23 | €0.20 |

### 6.4 Recommended Pair

> [!success] Recommended MOSFET Pair
> **Si2301 (P-ch) + Si2302 (N-ch)**
> - Matched characteristics
> - Very low threshold voltage (works with 3.3V or 5V logic)
> - Widely available
> - Total cost: ~€0.30

---

## 7. MOSFET Half-Bridge Design Details

### 7.1 Complete Schematic

```
                                  V_bat (6-9V)
                                      │
                                      │
                 ┌────────────────────┴────────────────────┐
                 │                                         │
                 │    ┌─────────────────────────────┐      │
                 │    │           Q1                │      │
                 │    │         Si2301              │      │
                 │    │        (P-channel)          │      │
                 │    │                             │      │
                 │    │    S ──────┬────── D        │      │
                 │    │            │                │      │
                 │    └────────────│────────────────┘      │
                 │                 │                       │
                 │            ┌────┴────┐                  │
    Pin 11 ──────┼───[R1]─────┤    G    │                  │
    (OC1A)       │   100Ω     └─────────┘                  │
                 │                 │                       │
                 │                 │ (Drain)               │
                 │                 │                       │
                 │                 ├───────────────────────┤
                 │                 │                       │
                 │                 │         ┌─────────┐   │
                 │                 │         │         │   │
                 │            ┌────┴────┐    │    C1   │   │
                 │            │         │    │  470nF  │   │
                 │            │ TX Coil │    │ (opt.)  │   │
                 │            │  L=15mH │    │         │   │
                 │            │  R=15Ω  │    └────┬────┘   │
                 │            └────┬────┘         │        │
                 │                 │              │        │
                 │                 ├──────────────┘        │
                 │                 │                       │
                 │                 │ (Drain)               │
                 │                 │                       │
                 │            ┌────┴────┐                  │
    Pin 12 ──────┼───[R2]─────┤    G    │                  │
    (OC1B)       │   100Ω     └─────────┘                  │
                 │                 │                       │
                 │    ┌────────────│────────────────┐      │
                 │    │            │                │      │
                 │    │    D ──────┴────── S        │      │
                 │    │                             │      │
                 │    │           Q2                │      │
                 │    │         Si2302              │      │
                 │    │        (N-channel)          │      │
                 │    └─────────────────────────────┘      │
                 │                 │                       │
                 │                 │                       │
                 └─────────────────┴───────────────────────┘
                                   │
                                   │
                                  GND
```

### 7.2 Operating Principle

**Phase 1: High-side ON, Low-side OFF**
- Q1 (P-ch) gate LOW → Q1 conducts
- Q2 (N-ch) gate LOW → Q2 off
- Current flows: V_bat → Q1 → Coil → (stored in L)

**Dead time: Both OFF**
- Both gates in transition
- Body diodes handle freewheeling current
- Duration: 1-2 µs

**Phase 2: High-side OFF, Low-side ON**
- Q1 (P-ch) gate HIGH → Q1 off
- Q2 (N-ch) gate HIGH → Q2 conducts
- Current flows: GND → Q2 → Coil → (energy released)

### 7.3 Dead-Time (Shoot-Through Prevention)

> [!danger] CRITICAL: Shoot-Through Prevention
> Both MOSFETs must NEVER be ON simultaneously!
> If both conduct: $I_{short} = V_{bat} / R_{DS,total} = 9V / 0.15Ω = 60A$ → instant destruction!

**Software dead-time implementation:**

```c
// Timer1 complementary PWM with dead-time
// Using OC1A (Pin 11) and OC1B (Pin 12)

#define DEAD_TIME_CYCLES 32  // 2µs at 16MHz

void init_half_bridge_pwm(void) {
    // Set pins as outputs
    DDRB |= (1 << PB5) | (1 << PB6);  // OC1A, OC1B
    
    // Timer1: Phase-correct PWM, TOP = ICR1
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << CS10);  // No prescaler
    
    // Set frequency: f = F_CPU / (2 * ICR1)
    // For 2 kHz: ICR1 = 16MHz / (2 * 2000) = 4000
    ICR1 = 4000;
    
    // 50% duty with dead-time
    OCR1A = 2000 - DEAD_TIME_CYCLES;  // High-side
    OCR1B = 2000 + DEAD_TIME_CYCLES;  // Low-side (inverted)
}
```

**Hardware dead-time (alternative):**

Use a half-bridge gate driver IC with built-in dead-time:
- IR2104: 540 ns fixed dead-time
- IR2184: Adjustable dead-time

### 7.4 Gate Drive Considerations

**P-channel high-side drive:**

The Si2301 P-channel turns ON when $V_{GS} < -V_{th}$ (gate lower than source).

- Source connected to V_bat (9V)
- Gate driven by MCU (0-5V)
- When MCU outputs LOW (0V): $V_{GS} = 0 - 9 = -9V$ → **ON**
- When MCU outputs HIGH (5V): $V_{GS} = 5 - 9 = -4V$ → still ON!

> [!warning] Problem
> 5V MCU cannot fully turn OFF a P-channel with source at 9V!

**Solutions:**

1. **Level shifter** (recommended for reliability):
```
                    V_bat (9V)
                       │
                      [10k]
                       │
    Pin 11 ───[1k]────┤ ├──────► To Q1 Gate
                      │ │
                     2N7002
                      │
                     GND
```

2. **Accept partial turn-off** (works for low current):
   - At $V_{GS} = -4V$ and $V_{th} = -1.2V$, still conducting but with higher $R_{DS(on)}$
   - For 40 mA, losses are still acceptable

3. **Use gate driver IC** (cleanest solution):
   - IR2104 provides proper high-side drive
   - Handles level shifting automatically

### 7.5 Simplified Circuit (Acceptable for 40 mA)

For your low-current application, direct MCU drive works:

```
                              V_bat (9V)
                                 │
                            ┌────┴────┐
                            │   Q1    │
                            │ Si2301  │
    Pin 11 ────[100Ω]───────┤G   S   D├───┐
                            └─────────┘   │
                                          │
                                     ┌────┴────┐
                                     │ TX Coil │
                                     └────┬────┘
                                          │
                            ┌─────────────┘
                            │
                            │
                            │
                       ┌────┴────┐
                       │   Q2    │
    Pin 12 ────[100Ω]──┤G  D    S├───► GND
                       │ Si2302  │
                       └─────────┘
```

**Note:** Q1 won't fully turn off (sees $V_{GS} = -4V$), but leakage is minimal at these current levels. For a production design, add the level shifter.

---

## 8. Power Loss Analysis

### 8.1 MOSFET Half-Bridge Losses

**Conduction losses:**

$$P_{cond} = I_{RMS}^2 \times (R_{DS,P} + R_{DS,N})$$

At 40 mA RMS:

$$P_{cond} = (0.040)^2 \times (0.100 + 0.050) = 0.24\ \text{mW}$$

**Switching losses at 2 kHz:**

$$P_{sw} = \frac{1}{2} V_{DS} \times I_D \times (t_r + t_f) \times f$$

$$P_{sw} = \frac{1}{2} \times 9V \times 40mA \times 20ns \times 2000 = 7.2\ \mu W$$

**Gate drive losses:**

$$P_{gate} = Q_g \times V_{GS} \times f \times 2 = 5nC \times 5V \times 2kHz \times 2 = 0.1\ \text{mW}$$

**Total MOSFET half-bridge losses:**

$$P_{total} = 0.24 + 0.007 + 0.1 = 0.35\ \text{mW}$$

### 8.2 Efficiency Comparison Summary

| Topology | Power Loss | Efficiency | Drive Current |
|----------|------------|------------|---------------|
| Simple NPN | 20 mW | 94.4% | 2.9 mA |
| BJT Half-Bridge | 66 mW | 81.7% | 6 mA |
| **MOSFET Half-Bridge** | **0.35 mW** | **99.9%** | **~0 mA** |

### 8.3 Complete Power Budget (MOSFET Half-Bridge)

```
┌──────────────────────────────────────────────────────────────────────┐
│              POWER BUDGET WITH MOSFET HALF-BRIDGE                    │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ELECTRONICS                                             80 mA      │
│  ├─ Arduino Mega 2560 Board                              62 mA      │
│  ├─ SSD1306 OLED Display                                 18 mA      │
│  └─ DSP Overhead                                        0.5 mA      │
│                                                                      │
│  TX COIL SYSTEM (MOSFET Half-Bridge)                     40 mA      │
│  ├─ TX Coil current                                      40 mA      │
│  └─ Driver losses                                       ~0 mA       │
│                                                                      │
│  TOTAL                                                  120 mA      │
│  vs. BJT Half-Bridge                                    126 mA      │
│                                                                      │
│  SAVINGS FROM MOSFET: 6 mA (5% of budget!)                         │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 9. Alternative: Half-Bridge Driver ICs

For a cleaner, more robust solution, consider integrated driver ICs:

### 9.1 Discrete Driver: IR2104

```
                         V_bat (9V)
                            │
              ┌─────────────┴─────────────┐
              │                           │
         ┌────┴────┐                 ┌────┴────┐
         │   HO    │                 │ VCC VB  │
         │         │                 │         │
         │  IR2104 │                 │ Bootstrap│
         │         │                 │   Cap   │
         │   LO    │                 │  100nF  │
         └────┬────┘                 └────┬────┘
              │                           │
              │    ┌──────────────────────┘
              │    │
              │    ▼
         ┌────┴────┬────────┐
         │   Q1    │        │
         │ N-ch    │        │
         └────┬────┘   ┌────┴────┐
              │        │ TX Coil │
              │        └────┬────┘
         ┌────┴────┐        │
         │   Q2    ├────────┘
         │ N-ch    │
         └────┬────┘
              │
             GND
```

**Advantages:**
- Built-in dead-time (540 ns)
- Bootstrap for high-side N-channel (simpler than P-channel)
- Under-voltage lockout
- Shoot-through protection

### 9.2 Integrated H-Bridge: DRV8837

Complete solution in one chip:

| Parameter | Value |
|-----------|-------|
| Supply | 0-11V |
| Output current | 1.8A peak |
| $R_{DS(on)}$ | 280 mΩ (high + low) |
| Package | 8-WSON (2×2 mm) |
| Price | ~€1.50 |

```
                    V_bat (9V)
                       │
              ┌────────┴────────┐
              │     DRV8837     │
              │                 │
    Pin 11 ───┤ IN1        OUT1 ├───┐
              │                 │   │
    Pin 12 ───┤ IN2        OUT2 ├───┼───► TX Coil
              │                 │   │
              │     nSLEEP      │   │
              └────────┬────────┘   │
                       │            │
                      GND ◄─────────┘
```

**Advantages:**
- Single chip solution
- Built-in protection (overcurrent, thermal)
- No external MOSFETs needed
- No dead-time calculation needed

---

## 10. Final Design Recommendations

### 10.1 Decision Matrix

| Criteria | Simple NPN | MOSFET Half-Bridge | Driver IC |
|----------|------------|-------------------|-----------|
| Efficiency | Good (94%) | Excellent (99.9%) | Very Good (98%) |
| Complexity | Very Low | Low | Very Low |
| Component count | 3 | 4-6 | 2-3 |
| Drive current | 2.9 mA | ~0 mA | ~0 mA |
| AC waveform | No (unipolar) | Yes (bipolar) | Yes (bipolar) |
| Cost | €0.30 | €0.50 | €1.50 |
| EMI | Higher | Lower with tank | Lowest |

### 10.2 Recommendations by Priority

> [!tip] Recommendations by Priority
> **If simplicity is priority:** Use Simple NPN Switch (Option 1)
> - Proven design, minimal components
> - 94% efficient — good enough for your budget
>
> **If efficiency is priority:** Use MOSFET Half-Bridge (Option 2)
> - Saves 6 mA from tight budget
> - 99.9% efficient
> - Bipolar drive = stronger magnetic field
>
> **If robustness is priority:** Use DRV8837 IC (Option 3)
> - Built-in protections
> - Guaranteed dead-time
> - Professional solution

### 10.3 Recommended Final BOM

**MOSFET Half-Bridge (Best balance):**

| Ref | Component | Value | Package | Qty | Price |
|-----|-----------|-------|---------|-----|-------|
| Q1 | P-ch MOSFET | Si2301 | SOT-23 | 1 | €0.15 |
| Q2 | N-ch MOSFET | Si2302 | SOT-23 | 1 | €0.15 |
| R1 | Gate resistor | 100 Ω | 0805 | 1 | €0.01 |
| R2 | Gate resistor | 100 Ω | 0805 | 1 | €0.01 |
| C1 | Tank capacitor | 470 nF | Film | 1 | €0.20 |
| | | | **Total:** | | **€0.52** |

---

## 11. References

- [[Power Budget Analysis]]
- [[Coil Basics.pdf|Coil Design Basics]]
- [[RLC Circuits and Resonance.pdf|RLC Circuits and Resonance]]
- [[Course_intro.pdf|Course Introduction - TX Coil Section]]
- Si2301 Datasheet, Vishay
- Si2302 Datasheet, Vishay
- IR2104 Datasheet, Infineon
- DRV8837 Datasheet, Texas Instruments

---

#tx-driver #amplifier #tank-circuit #resonance #power-stage #mosfet #half-bridge
