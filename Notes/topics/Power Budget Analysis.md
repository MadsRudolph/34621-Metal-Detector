# Power Budget Analysis

> **Requirement:** Metal detector must run for 100 minutes on a 9V battery (6LR61) with remaining voltage >6V.
> 
> See: [[kravspecifikation.pdf|Kravspecifikation]] requirements 4 & 5

---

## 1. Battery Characterization

### 1.1 Duracell MN1604 Specifications

> **Datasheet:** [[Literature/duracell_9volt.pdf|Duracell 9V Datasheet]]

|Parameter|Value|Conditions|
|---|---|---|
|Nominal Voltage|9.0 V|Open circuit|
|Fresh Voltage|9.4 - 9.6 V|No load|
|Operating Range|9.6 V - 4.8 V|Under load|
|Internal Impedance|1.7 Ω|@ 1 kHz, fresh|
|Internal Impedance|3-5 Ω|@ 1 kHz, discharged|
|Nominal Capacity|580 mAh|To 4.8V @ 25mA|
|Weight|45.6 g||
|Chemistry|Alkaline (Zn-MnO₂)||

### 1.2 Capacity vs. Discharge Rate

Alkaline batteries exhibit reduced effective capacity at higher discharge rates due to internal resistance and electrochemical limitations.

**Peukert's Equation (modified for alkaline):**

$$C_{eff} = C_{nom} \times \left(\frac{I_{nom}}{I_{actual}}\right)^{n-1}$$

Where:

- $C_{eff}$ = effective capacity (mAh)
- $C_{nom}$ = nominal capacity at standard rate (~500 mAh)
- $I_{nom}$ = nominal discharge rate (~25 mA for 9V alkaline)
- $I_{actual}$ = actual discharge current
- $n$ = Peukert exponent (~1.1 for alkaline)

**Example calculation at 100 mA:**

$$C_{eff} = 500 \times \left(\frac{25}{100}\right)^{0.1} = 500 \times 0.25^{0.1} = 500 \times 0.87 = 435\ \text{mAh}$$

### 1.3 Discharge Curve Analysis

From datasheet curves at 21°C:

|Current (mA)|Runtime to 6V|Effective Capacity|Efficiency|
|---|---|---|---|
|10|2400 min (40h)|400 mAh|80%|
|25|1080 min (18h)|450 mAh|90%|
|50|600 min (10h)|500 mAh|100%|
|100|210 min (3.5h)|350 mAh|70%|
|150|120 min (2h)|300 mAh|60%|
|200|75 min (1.25h)|250 mAh|50%|

### 1.4 Internal Resistance Effects

Battery terminal voltage under load:

$$V_{terminal} = V_{OC} - I \times R_{internal}$$

**Fresh battery ($R_{int} \approx 2\Omega$) at 100mA:**

$$V_{terminal} = 9.4V - 0.1A \times 2\Omega = 9.4V - 0.2V = 9.2V$$

**Discharged battery ($R_{int} \approx 4\Omega$) at 100mA:**

$$V_{terminal} = 7.0V - 0.1A \times 4\Omega = 7.0V - 0.4V = 6.6V$$

This explains why voltage drops faster near end-of-life.

---

## 2. Runtime Requirement Derivation

### 2.1 Requirement Statement

From kravspecifikation:

- Minimum runtime: $t_{min} = 100$ minutes
- End voltage: $V_{end} > 6.0V$

### 2.2 Maximum Current Calculation

**Method 1: Linear interpolation from discharge data**

From datasheet:

- At $I_1 = 100$ mA → $t_1 = 210$ min
- At $I_2 = 150$ mA → $t_2 = 120$ min

For $t_{target} = 100$ min, linear interpolation:

$$I_{max} = I_1 + \frac{(t_1 - t_{target})}{(t_1 - t_2)} \times (I_2 - I_1)$$

$$I_{max} = 100 + \frac{(210 - 100)}{(210 - 120)} \times (150 - 100)$$

$$I_{max} = 100 + \frac{110}{90} \times 50 = 100 + 61.1 = 161\ \text{mA}$$

**Method 2: Capacity-based calculation**

Using effective capacity at ~150 mA ($C_{eff} \approx 300$ mAh):

$$t = \frac{C_{eff}}{I} \Rightarrow I = \frac{C_{eff}}{t} = \frac{300\ \text{mAh}}{100\ \text{min}/60} = \frac{300}{1.67} = 180\ \text{mA}$$

**Method 3: Conservative estimate accounting for uncertainties**

Factors reducing actual capacity:

- Temperature variation: -10% (if cold)
- Battery aging: -5%
- Manufacturing variation: -5%
- Self-discharge: -2%

Combined derating factor: $0.9 \times 0.95 \times 0.95 \times 0.98 = 0.79$

$$I_{conservative} = I_{max} \times 0.79 = 161 \times 0.79 = 127\ \text{mA}$$

> **Design Maximum: $I_{max} = 120$ mA** (rounded down for margin)

### 2.3 Design Target with Safety Margin

Applying 1.2× safety factor for measurement uncertainty:

$$I_{target} = \frac{I_{max}}{1.2} = \frac{120}{1.2} = 100\ \text{mA}$$

> **Design Target: $I_{system} \leq 100$ mA**

---

## 3. Current Measurement Analysis

### 3.1 Test Configuration

|Parameter|Value|
|---|---|
|Date|January 2025|
|Shunt Resistor|$R_{shunt} = 1.00\ \Omega \pm 1%$|
|Measurement Device|Analog Discovery 3 Oscilloscope|
|AD3 Resolution|14-bit, ±25mV range|
|Sampling Rate|1 kHz|
|Averaging Window|10 seconds|
|Test Program|Prime number calculator|
|DUT|Arduino Mega 2560 + SSD1306 OLED|

### 3.2 Measurement Theory

**Shunt resistor method:**

Current through shunt creates voltage drop:

$$V_{shunt} = I_{load} \times R_{shunt}$$

Solving for current:

$$I_{load} = \frac{V_{shunt}}{R_{shunt}}$$

With $R_{shunt} = 1.00\ \Omega$:

$$I_{load}\ [\text{mA}] = V_{shunt}\ [\text{mV}]$$

This 1:1 relationship simplifies calculations.

### 3.3 Raw Measurement Data

|Statistic|$V_{shunt}$ (mV)|$I_{load}$ (mA)|
|---|---|---|
|Mean (μ)|79.49|79.49|
|Maximum|91.71|91.71|
|Minimum|66.49|66.49|
|Std Dev (σ)|5.12|5.12|
|Peak-to-Peak|25.22|25.22|

### 3.4 Measurement Uncertainty Analysis

**Sources of uncertainty:**

1. **Shunt resistor tolerance:** ±1% $$u_R = 0.01 \times 79.49 = 0.79\ \text{mA}$$
    
2. **AD3 voltage accuracy:** ±0.5% + 2mV offset $$u_V = 0.005 \times 79.49 + 2 \times 1 = 0.40 + 2 = 2.4\ \text{mA}$$
    
3. **Temperature coefficient of shunt:** ~50 ppm/°C At ΔT = 10°C: $$u_T = 50 \times 10^{-6} \times 10 \times 79.49 = 0.04\ \text{mA}$$
    
4. **Statistical uncertainty:** (σ/√n for n=10000 samples) $$u_{stat} = \frac{5.12}{\sqrt{10000}} = 0.05\ \text{mA}$$
    

**Combined uncertainty (RSS method):**

$$u_{combined} = \sqrt{u_R^2 + u_V^2 + u_T^2 + u_{stat}^2}$$

$$u_{combined} = \sqrt{0.79^2 + 2.4^2 + 0.04^2 + 0.05^2} = \sqrt{0.62 + 5.76 + 0.002 + 0.003}$$

$$u_{combined} = \sqrt{6.39} = 2.53\ \text{mA}$$

**Measurement result:**

$$I_{measured} = 79.5 \pm 2.5\ \text{mA}\ (k=1)$$

Or with 95% confidence (k=2):

$$I_{measured} = 79.5 \pm 5.1\ \text{mA (95\% CI)}$$

### 3.5 Current Fluctuation Analysis

The observed 25.2 mA peak-to-peak variation has identifiable sources:

**OLED I2C Communication:**

- Data rate: 400 kHz I2C
- Bytes per frame: ~1024 bytes (8 pages × 128 columns)
- Transmission time: 1024 × 9 bits / 400 kHz = 23 ms
- Update period: ~83 ms (12 Hz)
- I2C duty cycle: 23/83 = 28%

During I2C transmission, current increases due to:

- SCL/SDA line charging/discharging
- MCU I2C peripheral active
- OLED controller receiving data

Estimated $\Delta I_{I2C} \approx 10-15$ mA during bursts.

**CPU Load Variation:**

- Prime checking: intensive for large primes
- Display string formatting: moderate
- Idle waiting: minimal

Estimated $\Delta I_{CPU} \approx 5-10$ mA variation.

**Combined:** 15-25 mA variation matches observation ✓

---

## 4. Arduino Mega 2560 Power Analysis

### 4.1 Board Block Diagram

```
                    ┌─────────────────────────────────────────┐
                    │         Arduino Mega 2560               │
                    │                                         │
    9V Battery ────►│──► NCP1117 ──► 5V Rail ──┬──► ATmega2560│
         │          │    (LDO)        │        │              │
         │          │                 │        ├──► ATmega16U2│
         │          │                 │        │   (USB chip) │
         │          │                 │        ├──► Power LED │
         │          │                 │        └──► OLED      │
         │          │                 │                       │
         └──────────│─────────────────┴─► 3.3V Regulator     │
                    │                     (LP2985)            │
                    └─────────────────────────────────────────┘
```

### 4.2 Component-by-Component Analysis

#### 4.2.1 ATmega2560 Microcontroller

**From ATmega2560 Datasheet, Section 35.2:**

|Mode|Conditions|Current|
|---|---|---|
|Active|16 MHz, 5V|14 mA (typ), 25 mA (max)|
|Idle|16 MHz, 5V|5.5 mA|
|Power-save|32 kHz RTC, 5V|0.9 mA|
|Power-down|WDT enabled, 5V|15 µA|
|Power-down|WDT disabled, 5V|0.3 µA|

**Peripheral current additions:**

|Peripheral|Additional Current|Notes|
|---|---|---|
|ADC|0.3 mA|During conversion|
|Analog Comparator|0.07 mA|When enabled|
|Brown-out Detector|0.02 mA|Always on by default|
|Watchdog Timer|0.01 mA|If enabled|
|All Timers (0,1,2,3,4,5)|0.5 mA|All running|
|USART0|0.1 mA|Active|
|TWI (I2C)|0.1 mA|Active|
|SPI|0.1 mA|Active|

**Estimated ATmega2560 total (our application):**

$$I_{MCU} = I_{active} + I_{ADC} + I_{timers} + I_{TWI} + I_{USART}$$

$$I_{MCU} = 14 + 0.3 + 0.5 + 0.1 + 0.1 = 15\ \text{mA (typical)}$$

Using datasheet maximum for safety: **$I_{MCU} = 20$ mA**

#### 4.2.2 ATmega16U2 USB Interface

The Arduino Mega uses ATmega16U2 for USB-to-Serial conversion.

**From ATmega16U2 datasheet:**

|Mode|Current|
|---|---|
|Active (USB connected)|25 mA|
|Active (USB idle)|12 mA|
|USB disconnected|10-12 mA|

The chip remains powered even when USB is disconnected!

**$I_{USB_chip} = 12$ mA**

#### 4.2.3 Power LED

Green LED with current-limiting resistor.

**Circuit:** 5V → LED ($V_f \approx 2.1V$) → R → GND

For 1kΩ resistor: $$I_{LED} = \frac{V_{CC} - V_f}{R} = \frac{5V - 2.1V}{1000\Omega} = 2.9\ \text{mA}$$

For 330Ω resistor (some board revisions): $$I_{LED} = \frac{5V - 2.1V}{330\Omega} = 8.8\ \text{mA}$$

**$I_{LED} = 3-9$ mA** (depends on board revision)

Using typical value: **$I_{LED} = 8$ mA**

#### 4.2.4 Voltage Regulator Analysis

The NCP1117 is a linear (LDO) regulator.

**Linear regulator fundamentals:**

Power in = Power out + Power dissipated

$$P_{in} = P_{out} + P_{loss}$$

For an ideal linear regulator, $I_{in} = I_{out}$, but there's also quiescent current:

$$I_{in} = I_{out} + I_q$$

**NCP1117 specifications:**

- Quiescent current: $I_q = 5$ mA (typical), 10 mA (max)
- Dropout voltage: 1.2V @ 800mA

**Power dissipation in regulator:**

At $V_{in} = 9V$, $V_{out} = 5V$, $I_{out} = 60mA$:

$$P_{loss} = (V_{in} - V_{out}) \times I_{out} = 4V \times 60mA = 240\ \text{mW}$$

This power is wasted as heat in the regulator.

**Regulator overhead: $I_q = 5$ mA**

#### 4.2.5 SSD1306 OLED Display

**From SSD1306 datasheet:**

|Parameter|Value|
|---|---|
|Supply voltage|3.3V - 5V|
|Standby current|10 µA|
|Operating current (50% pixels)|8-10 mA|
|Operating current (all pixels ON)|20 mA|

For typical text display (~30% pixels lit):

**$I_{OLED} = 15$ mA**

#### 4.2.6 3.3V Regulator

LP2985 provides 3.3V for some analog circuitry.

- Quiescent current: 1 mA
- Typical load: <1 mA

**$I_{3V3} = 2$ mA**

### 4.3 Total Board Current Summary

|Component|Current (mA)|Notes|
|---|---|---|
|ATmega2560 (active)|20|Conservative estimate|
|ATmega16U2 (USB chip)|12|Always on|
|Power LED|8|Green LED|
|NCP1117 quiescent|5|LDO overhead|
|SSD1306 OLED|15|~30% pixels|
|3.3V regulator system|2|LP2985 + load|
|PCB leakage|1|Capacitors, traces|
|**Subtotal (5V rail)**|**63 mA**||

**Measured vs Calculated:**

We measured 79.5 mA from the 9V supply, but calculated only 63 mA.

For a linear regulator: $I_{battery} = I_{5V,load} + I_{quiescent} = 63 + 5 = 68$ mA

**Remaining discrepancy:** 79.5 - 68 = 11.5 mA

Possible explanations:

1. Test program CPU load higher than "active" baseline
2. I2C bus pull-up currents during heavy traffic
3. Higher actual OLED current than datasheet typical
4. ATmega2560 running closer to maximum (25mA) than typical (14mA)

**Revised estimate with measured data:**

|Component|Current (mA)|
|---|---|
|ATmega2560 (measured load)|25|
|ATmega16U2|12|
|Power LED|8|
|Regulator quiescent|5|
|SSD1306 OLED (active refresh)|18|
|3.3V system|2|
|I2C bus pull-ups during traffic|3|
|PCB/misc|2|
|**Total from 9V**|**75 mA**|

Uncertainty: ±5 mA → Range: 70-80 mA ✓ **Matches measurement!**

---

## 5. DSP Overhead — Detailed Analysis

### 5.1 Timer1 (TX Signal Generation)

**Configuration:**

- Mode: CTC (Clear Timer on Compare)
- Output: Toggle OC1A on compare match
- Frequency: 2 kHz square wave

**Power analysis:**

From ATmega2560 datasheet, Timer1 adds approximately:

$$\Delta I_{Timer1} \approx 0.1\ \text{mA}$$

The actual pin toggle (charging load capacitance):

$$I_{toggle} = C_{load} \times V_{CC} \times f = 20pF \times 5V \times 4000Hz = 0.4\ \mu\text{A}$$

**Total Timer1 overhead: ~0.1 mA** (negligible)

### 5.2 Timer3 (Sampling ISR)

**Configuration:**

- Mode: CTC
- Frequency: 8 kHz interrupt
- ISR execution time: ~100 cycles = 6.25 µs @ 16MHz

**Duty cycle calculation:**

$$D = \frac{t_{ISR}}{T_{period}} = \frac{6.25\ \mu s}{125\ \mu s} = 5\%$$

**Current impact:**

If MCU draws 20mA active vs 5.5mA idle:

$$\Delta I_{ISR} = (I_{active} - I_{idle}) \times D = (20 - 5.5) \times 0.05 = 0.725\ \text{mA}$$

But since main loop is also active (not idle), the marginal increase is essentially zero.

**Total Timer3 overhead: ~0.2 mA** (timer peripheral itself)

### 5.3 ADC Conversion

**Configuration:**

- Resolution: 10-bit
- Clock: 125 kHz (16 MHz / 128)
- Conversion time: 13 ADC cycles = 104 µs
- Sample rate: 8 kHz (in ISR)

104 µs conversion time vs 125 µs sample period means ADC is active 83% of the time.

**ADC current:**

From datasheet: ADC adds 0.3 mA when active.

At 83% duty cycle:

$$I_{ADC} = 0.3\ \text{mA} \times 0.83 = 0.25\ \text{mA}$$

**Total ADC overhead: ~0.3 mA**

### 5.4 DFT Computation

**Algorithm:** Single-bin DFT with 4× oversampling optimization

For k corresponding to 2 kHz with 8 kHz sampling (k=N/4):

$$e^{-j2\pi n/4} = {1, -j, -1, j}$$

So: cos = {1, 0, -1, 0}, sin = {0, 1, 0, -1}

**Implementation (no multiplications!):**

```c
real_sum = x[0] - x[2] + x[4] - x[6] + ...  // Only additions
imag_sum = x[1] - x[3] + x[5] - x[7] + ...
```

**Power impact:**

The CPU is already running. DFT computation doesn't increase current — it just uses CPU cycles.

**DFT overhead: 0 mA** (no additional current, just CPU time)

### 5.5 IIR Filter

**Algorithm:**

$$y[n] = \alpha \cdot x[n] + (1-\alpha) \cdot y[n-1]$$

With α = 0.1:

$$y[n] = 0.1 \cdot x[n] + 0.9 \cdot y[n-1]$$

**IIR overhead: 0 mA** (just CPU time)

### 5.6 Total DSP Overhead

|Component|Current (mA)|
|---|---|
|Timer1 peripheral|0.1|
|Timer3 peripheral|0.1|
|ADC (83% active)|0.3|
|DFT computation|0|
|IIR filter|0|
|**Total DSP overhead**|**0.5 mA**|

---

## 6. Revised Power Budget

### 6.1 Final Current Breakdown

|Component|Current (mA)|Confidence|
|---|---|---|
|**Measured baseline**|**79.5**|High (measured)|
|**DSP additions**|**0.5**|High (calculated)|
|**Total Electronics**|**80 mA**|High|

### 6.2 Available Current for TX Coil

$$I_{TX,available} = I_{max} - I_{electronics}$$

$$I_{TX,available} = 120\ \text{mA} - 80\ \text{mA} = 40\ \text{mA}$$

With 10% margin for measurement uncertainty:

$$I_{TX,design} = 40 \times 0.9 = 36\ \text{mA}$$

> **Design target for TX coil: 35-40 mA**

### 6.3 Available Power for TX Coil

$$P_{TX} = V_{supply} \times I_{TX}$$

|Battery State|$V_{supply}$ (V)|$I_{TX}$ (mA)|$P_{TX}$ (mW)|
|---|---|---|---|
|Fresh|9.0|40|**360**|
|75% capacity|8.0|40|**320**|
|50% capacity|7.5|40|**300**|
|25% capacity|7.0|40|**280**|
|End of life|6.5|40|**260**|
|Cutoff|6.0|40|**240**|

> **TX Coil power budget: 240-360 mW**

---

## 7. TX Coil Design Calculations

### 7.1 Electrical Model of Inductor

A real inductor is modeled as:

```
        ┌───────┐
    ────┤  R_dc ├────┬────────
        └───────┘    │
                    ┌┴┐
                    │ │ L
                    │ │
                    └┬┘
                     │
    ─────────────────┴────────
```

**Impedance:**

$$Z = R_{dc} + j\omega L = R_{dc} + j2\pi f L$$

**Magnitude:**

$$|Z| = \sqrt{R_{dc}^2 + (2\pi f L)^2}$$

**Phase:**

$$\phi = \arctan\left(\frac{2\pi f L}{R_{dc}}\right)$$

### 7.2 Required Impedance Calculation

For target current $I_{coil}$ at supply voltage $V$:

$$|Z_{coil}| = \frac{V_{supply}}{I_{coil}}$$

| $V_{supply}$ | $I_{coil}$ | $|Z|$ | |--------------|------------|-------| | 9.0 V | 40 mA | 225 Ω | | 7.5 V | 40 mA | 187.5 Ω | | 6.0 V | 40 mA | 150 Ω |

**Design target: $|Z| \approx 190\ \Omega$** (for mid-life battery voltage)

### 7.3 Inductance Calculation

At 2 kHz, assuming $R_{dc} \ll X_L$:

$$|Z| \approx X_L = 2\pi f L$$

$$L = \frac{|Z|}{2\pi f} = \frac{190}{2\pi \times 2000} = \frac{190}{12566} = 15.1\ \text{mH}$$

**Target TX coil inductance: L = 15 mH**

### 7.4 Verification of $R_{dc} \ll X_L$ Assumption

**Wire resistance per length:**

For 0.3 mm diameter (AWG 28-29) enameled copper wire:

$$\rho_{Cu} = 1.68 \times 10^{-8}\ \Omega\cdot m$$

$$R_{wire} = \frac{\rho \cdot l}{A} = \frac{1.68 \times 10^{-8} \times l}{\pi (0.15\times10^{-3})^2}$$

$$R_{wire} = 0.238\ \Omega/m \approx 0.24\ \Omega/m$$

**Estimating turns for 15 mH coil:**

Using empirical formula for single-layer solenoid:

$$L = \frac{\mu_0 N^2 A}{l}$$

For a coil with diameter 150 mm, solving for N to get 15 mH:

$$N = \sqrt{\frac{L \times l}{\mu_0 \times A}} \approx 130\ \text{turns}$$

**Wire length for 130 turns:**

- Circumference per turn: $C = \pi \times D = \pi \times 0.15 = 0.471$ m
- Total length: $l_{wire} = N \times C = 130 \times 0.471 = 61$ m

**DC Resistance:**

$$R_{dc} = 0.24\ \Omega/m \times 61\ m = 14.6\ \Omega$$

**Verification:**

$$\frac{X_L}{R_{dc}} = \frac{2\pi \times 2000 \times 0.015}{14.6} = \frac{188}{14.6} = 12.9$$

Since $X_L$ is 13× larger than $R_{dc}$, the approximation $|Z| \approx X_L$ is reasonable (error ~3%).

**Actual impedance:**

$$|Z| = \sqrt{14.6^2 + 188^2} = \sqrt{213 + 35344} = 188.6\ \Omega$$

### 7.5 Coil Design Summary

|Parameter|Value|Notes|
|---|---|---|
|Target inductance|15 mH||
|Coil diameter|150 mm|Typical for handheld detector|
|Number of turns|~130|Estimated|
|Wire gauge|0.3 mm (AWG 28)|Enameled copper|
|Wire length|~61 m||
|DC resistance|~15 Ω||
|Reactance @ 2kHz|188 Ω|$X_L = 2\pi fL$|
|Total impedance|~189 Ω||
|Current @ 7.5V|39.7 mA|$I = V/Z$|
|Power @ 7.5V|298 mW|$P = V^2/Z$|

---

## 8. TX Coil Driver Circuit

> **Detailed design:** See [[TX Driver and Tank Circuit Design]]

### 8.1 Amplifier Topology Selection

Two main options were considered for driving the TX coil:

|Aspect|Class D (Switching)|Class AB (Linear)|
|---|---|---|
|**Efficiency**|90-95%|50-70%|
|**Heat dissipation**|Very low (~8 mW)|High (~140 mW)|
|**Power to coil**|~340 mW|~220 mW|
|**Complexity**|Low (3 components)|Medium (4-8 components)|
|**Waveform**|Square (filtered by coil)|Sine capable|

**Efficiency comparison at 40 mA, 9V:**

**Class D:** $$\eta_D = \frac{P_{coil}}{P_{total}} = \frac{360 - 8}{360} = 97.8\%$$

**Class AB:** $$\eta_{AB} \approx 65\% \Rightarrow P_{wasted} = 126\ \text{mW as heat}$$

> **Selected: Class D (Switching)** — Higher efficiency means more power to coil and better detection range.

### 8.2 NPN Transistor Switch Design

**Circuit topology:**

```
                              V_bat (6-9V)
                                 │
                            ┌────┴────┐
                            │ TX Coil │
                            │ L=15mH  │
                            │ R=15Ω   │
                            └────┬────┘
                                 │
                            ┌────┴────┐
                            │   D1    │  1N4148 (Flyback)
                            └────┬────┘
                                 │
                                 ▼ Collector
                           ┌──────────┐
    Pin 11 ────[R_b]──────►│   Q1     │
    (2 kHz)                │ 2N2222A  │
                           └────┬─────┘
                                │ Emitter
                                ▼
                               GND
```

### 8.3 Transistor Selection: 2N2222A

|Parameter|Value|Requirement|
|---|---|---|
|$I_C$ (max)|800 mA|> 40 mA ✓|
|$V_{CEO}$|40 V|> 9V ✓|
|$h_{FE}$ (β)|75-300|Used for base calc|
|$V_{CE(sat)}$|0.3 V @ 150mA|Low saturation|
|$f_T$|300 MHz|>> 2 kHz ✓|
|$P_D$|625 mW|>> 8 mW ✓|

### 8.4 Base Resistor Calculation

**Step 1: Minimum base current for saturation**

$$I_{B,min} = \frac{I_C}{\beta_{min}} = \frac{40\ \text{mA}}{75} = 0.53\ \text{mA}$$

**Step 2: Design base current with overdrive factor**

For fast switching and guaranteed saturation, use 5× overdrive:

$$I_B = 5 \times I_{B,min} = 5 \times 0.53 = 2.67\ \text{mA}$$

**Step 3: Calculate base resistor**

$$R_B = \frac{V_{pin} - V_{BE}}{I_B} = \frac{5V - 0.7V}{2.67\ \text{mA}} = 1610\ \Omega$$

**Step 4: Select standard value**

Use **$R_B = 1.5\ k\Omega$** (E24 series)

**Verify:**

$$I_B = \frac{5V - 0.7V}{1500\ \Omega} = 2.87\ \text{mA}$$

This is well under Arduino's 20 mA limit ✓

### 8.5 Flyback Diode Analysis

**Problem:** When transistor turns off, inductor current cannot change instantaneously.

$$V_L = L \frac{di}{dt}$$

**Without protection, worst case** (turn-off in 100 ns):

$$V_{spike} = L \frac{\Delta i}{\Delta t} = 15\ \text{mH} \times \frac{40\ \text{mA}}{100\ \text{ns}} = 6000\ V$$

This would destroy the transistor!

**With flyback diode (1N4148):**

The diode clamps the voltage to $V_{bat} + V_f \approx 9 + 0.7 = 9.7$ V

### 8.6 Power Dissipation Analysis

**Transistor dissipation in saturation:** $V_{CE(sat)} \approx 0.2$ V

$$P_Q = V_{CE} \times I_C = 0.2V \times 40\ \text{mA} = 8\ \text{mW}$$

Well under 625 mW rating ✓ No heatsink required.

**Base resistor dissipation:**

$$P_{R_B} = I_B^2 \times R_B = (2.87\ \text{mA})^2 \times 1500\ \Omega = 12.4\ \text{mW}$$

Standard 1/8W (125 mW) resistor is adequate ✓

### 8.7 Driver Efficiency Summary

|Parameter|Value|
|---|---|
|Input power (from battery)|$9V \times 40mA = 360$ mW|
|Transistor loss|8 mW|
|Base resistor loss|12 mW|
|**Total driver loss**|**20 mW**|
|Power to coil|340 mW|
|**Driver efficiency**|**94.4%**|

### 8.8 Optional: Resonant Tank Circuit

For sine wave output (lower EMI), add LC tank circuit:

```
    V_bat ──► [R_lim 180Ω] ──► [C 470nF] ──► [TX Coil] ──► Transistor ──► GND
```

**Resonant frequency:**

$$f_0 = \frac{1}{2\pi\sqrt{LC}} = \frac{1}{2\pi\sqrt{0.015 \times 470 \times 10^{-9}}} = 1897\ \text{Hz}$$

Adjust Timer1 OCR1A = 4215 for 1897 Hz to match.

See [[TX Driver and Tank Circuit Design]] for full tank circuit analysis.

### 8.9 Complete Driver BOM

|Component|Value|Package|Qty|Notes|
|---|---|---|---|---|
|Q1|2N2222A|TO-92|1|Or BC547, 2N3904|
|$R_B$|1.5 kΩ|0805|1|1/8W minimum|
|D1|1N4148|DO-35|1|Fast recovery flyback|

**Optional for tank circuit:**

|Component|Value|Package|Notes|
|---|---|---|---|
|C1|470 nF|Film|50V min, polyester|
|R_lim|180 Ω|0805|Current limiting|

---

## 9. Detection Range Analysis

### 9.1 Magnetic Dipole Model

A circular coil carrying current I can be modeled as a magnetic dipole:

$$m = N \times I \times A = N \times I \times \pi r^2$$

**For our TX coil:**

$$m = 130 \times 0.040 \times \pi \times 0.075^2 = 0.092\ A \cdot m^2$$

### 9.2 Magnetic Field vs. Distance

On-axis magnetic field at distance $z \gg r$:

$$B_z = \frac{\mu_0}{4\pi} \times \frac{2m}{z^3}$$

**At z = 50 mm (detection requirement):**

$$B_z = 10^{-7} \times \frac{2 \times 0.092}{0.050^3} = 147\ \mu T$$

### 9.3 Skin Depth in Iron at 2 kHz

$$\delta = \sqrt{\frac{2}{\omega \mu \sigma}} = 0.36\ \text{mm}$$

Since skin depth (0.36 mm) << target radius (15 mm), the entire cross-section contributes.

### 9.4 Expected Signal Level

|Coil Power|Target @ 50mm|Signal Level|
|---|---|---|
|100 mW|Iron cylinder|~0.1 mV at RX|
|300 mW|Iron cylinder|~0.3 mV at RX|
|300 mW|Copper coin|~0.5 mV at RX|

With 100× amplification: ~30 mV (6 LSB with 10-bit ADC)

> **Assessment:** Detection at 50mm should be achievable with careful analog design

---

## 10. Complete Power Budget Summary

```
┌──────────────────────────────────────────────────────────────────────┐
│                     COMPLETE POWER BUDGET                            │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  BATTERY: Duracell MN1604 @ 100mA load → ~350 mAh to 6V             │
│  REQUIRED RUNTIME: 100 min = 1.67 hours                             │
│  MAXIMUM CURRENT: 120 mA (with derating)                            │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ELECTRONICS (Measured + Calculated)                     80 mA      │
│  ├─ Arduino Mega 2560 Board                              62 mA      │
│  │   ├─ ATmega2560 (active, all peripherals)   25 mA               │
│  │   ├─ ATmega16U2 (USB interface)             12 mA               │
│  │   ├─ Power LED                               8 mA               │
│  │   ├─ NCP1117 regulator quiescent             5 mA               │
│  │   ├─ 3.3V subsystem                          2 mA               │
│  │   └─ PCB leakage, decoupling                10 mA               │
│  ├─ SSD1306 OLED Display                                 18 mA      │
│  └─ DSP Overhead                                        0.5 mA      │
│      ├─ Timer1 + Timer3                        0.2 mA              │
│      └─ ADC conversions                        0.3 mA              │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TX COIL SYSTEM (Class D Driver)                         40 mA      │
│  ├─ TX Coil (L=15mH, Z=189Ω @ 7.5V)                      39 mA      │
│  ├─ Driver transistor loss                              <0.5 mA     │
│  └─ Base resistor (1.5kΩ, 50% duty)                      ~1 mA      │
│                                                                      │
│  Driver Efficiency: 94.4%                                           │
│  Power to coil: 340 mW (of 360 mW input)                           │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TOTAL SYSTEM CURRENT                                   120 mA      │
│  MARGIN                                                   0 mA      │
│                                                                      │
│  STATUS: ⚠️  TIGHT - Consider optimizations                         │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

---

## 11. Optimization Options

**Priority 1: Remove Power LED** → saves 8 mA

- Cut PWR LED jumper or desolder LED
- Zero functional impact

**Priority 2: Reduce OLED refresh** (12Hz → 4Hz) → saves 3-5 mA

- Minimal user impact

**After optimizations: ~12 mA margin**

### 11.1 Why Class D Driver Matters

If we had chosen Class AB instead of Class D:

|Parameter|Class D (chosen)|Class AB (rejected)|
|---|---|---|
|Driver efficiency|94%|65%|
|Power wasted as heat|20 mW|126 mW|
|Power to coil|340 mW|234 mW|
|Detection performance|Better|Worse|
|Heat management|None needed|May need heatsink|

The Class D topology delivers **45% more power** to the coil!

---

## 12. Verification Test Plan

### 12.1 Component Tests

|Test|Setup|Pass Criteria|
|---|---|---|
|Arduino + OLED|AD3 @ 5V|< 85 mA|
|TX driver alone|Scope + DMM|35-45 mA|
|Full electronics|Shunt + battery|< 85 mA|
|Full system|Shunt + battery|< 125 mA|

### 12.2 100-Minute Runtime Test Log

|Time (min)|I (mA)|$V_{bat}$ (V)|Status|
|---|---|---|---|
|0|||Start|
|25||||
|50||||
|75||||
|**100**|||**V > 6.0V?**|

---

## 13. References

1. [[Literature/duracell_9volt.pdf|Duracell 9V Datasheet]]
2. [[kravspecifikation.pdf|Kravspecifikation]]
3. [[Coil Basics.pdf|Coil Design Fundamentals]]
4. [[TX Driver and Tank Circuit Design]]
5. ATmega2560 Datasheet, Microchip Technology
6. 2N2222A Datasheet, ON Semiconductor
7. SSD1306 Datasheet, Solomon Systech

---

#power #calculations #coil-design #driver #measurements #requirements