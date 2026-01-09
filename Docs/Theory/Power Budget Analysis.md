# Power Budget Analysis

> [!abstract] Core Requirement
> Metal detector must run for **100 minutes** on a 9V battery (6LR61) with remaining voltage **>6V**.
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
|10|2400 min (40h)|400 mAh|80\%|
|25|1080 min (18h)|450 mAh|90\%|
|50|600 min (10h)|500 mAh|100\%|
|100|210 min (3.5h)|350 mAh|70\%|
|150|120 min (2h)|300 mAh|60\%|
|200|75 min (1.25h)|250 mAh|50\%|

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

- Temperature variation: -10\% (if cold)
- Battery aging: -5\%
- Manufacturing variation: -5\%
- Self-discharge: -2\%

Combined derating factor: $0.9 \times 0.95 \times 0.95 \times 0.98 = 0.79$

$$I_{conservative} = I_{max} \times 0.79 = 161 \times 0.79 = 127\ \text{mA}$$

> [!danger] Design Maximum
> $I_{max} = 120$ mA (rounded down for margin)

### 2.3 Design Target with Safety Margin

Applying 1.2× safety factor for measurement uncertainty:

$$I_{target} = \frac{I_{max}}{1.2} = \frac{120}{1.2} = 100\ \text{mA}$$

> [!success] Design Target
> $I_{system} \leq 100$ mA

---

## 3. Current Measurement Analysis

### 3.1 Test Configuration

|Parameter|Value|
|---|---|
|Date|January 2025|
|Shunt Resistor|$R_{shunt} = 1.00\ \Omega \pm 1\%$|
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

1. **Shunt resistor tolerance:** ±1\% $$u_R = 0.01 \times 79.49 = 0.79\ \text{mA}$$
    
2. **AD3 voltage accuracy:** ±0.5\% + 2mV offset $$u_V = 0.005 \times 79.49 + 2 \times 1 = 0.40 + 2 = 2.4\ \text{mA}$$
    
3. **Temperature coefficient of shunt:** ~50 ppm/°C At ΔT = 10°C: $$u_T = 50 \times 10^{-6} \times 10 \times 79.49 = 0.04\ \text{mA}$$
    
4. **Statistical uncertainty:** (σ/√n for n=10000 samples) $$u_{stat} = \frac{5.12}{\sqrt{10000}} = 0.05\ \text{mA}$$
    

**Combined uncertainty (RSS method):**

$$u_{combined} = \sqrt{u_R^2 + u_V^2 + u_T^2 + u_{stat}^2}$$

$$u_{combined} = \sqrt{0.79^2 + 2.4^2 + 0.04^2 + 0.05^2} = \sqrt{0.62 + 5.76 + 0.002 + 0.003}$$

$$u_{combined} = \sqrt{6.39} = 2.53\ \text{mA}$$

**Measurement result:**

$$I_{measured} = 79.5 \pm 2.5\ \text{mA}\ (k=1)$$

Or with 95\% confidence (k=2):

$$I_{measured} = 79.5 \pm 5.1\ \text{mA (95\% CI)}$$

### 3.5 Current Fluctuation Analysis

The observed 25.2 mA peak-to-peak variation has identifiable sources:

**OLED I2C Communication:**

- Data rate: 400 kHz I2C
- Bytes per frame: ~1024 bytes (8 pages × 128 columns)
- Transmission time: 1024 × 9 bits / 400 kHz = 23 ms
- Update period: ~83 ms (12 Hz)
- I2C duty cycle: 23/83 = 28\%

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
|Operating current (50\% pixels)|8-10 mA|
|Operating current (all pixels ON)|20 mA|

For typical text display (~30\% pixels lit):

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
|SSD1306 OLED|15|~30\% pixels|
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

104 µs conversion time vs 125 µs sample period means ADC is active 83\% of the time.

**ADC current:**

From datasheet: ADC adds 0.3 mA when active.

At 83\% duty cycle:

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
|ADC (83\% active)|0.3|
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

With 10\% margin for measurement uncertainty:

$$I_{TX,design} = 40 \times 0.9 = 36\ \text{mA}$$

> [!tip] Design Target for TX Coil
> **35-40 mA** available for TX coil drive

### 6.3 Available Power for TX Coil

$$P_{TX} = V_{supply} \times I_{TX}$$

|Battery State|$V_{supply}$ (V)|$I_{TX}$ (mA)|$P_{TX}$ (mW)|
|---|---|---|---|
|Fresh|9.0|40|**360**|
|75\% capacity|8.0|40|**320**|
|50\% capacity|7.5|40|**300**|
|25\% capacity|7.0|40|**280**|
|End of life|6.5|40|**260**|
|Cutoff|6.0|40|**240**|

> [!info] TX Coil Power Budget
> **240-360 mW** available (varies with battery voltage)

---

## 7. TX Coil Design Calculations

> **Detailed coil designs:** See [[Search Coil Design]]

### 7.1 Wire Specification

|Parameter|Value|Notes|
|---|---|---|
|**Wire diameter**|**0.52 mm**|AWG 24 equivalent|
|Wire type|Enameled copper|Magnet wire|
|Resistance per meter|**0.079 Ω/m**||
|Current capacity|1.2 A|Well above 40 mA|

### 7.2 Wire Comparison

|Diameter|AWG|R (Ω/m)|61m coil R_dc|Q factor|
|---|---|---|---|---|
|0.30 mm|28|0.24|14.6 Ω|12.9|
|0.40 mm|26|0.14|8.5 Ω|22.1|
|**0.52 mm**|**24**|**0.079**|**4.8 Ω**|**39.2**|
|0.65 mm|22|0.05|3.1 Ω|60.8|

**Selected: 0.52 mm** — Best balance of low resistance, reasonable coil size, and ease of handling.

### 7.3 Electrical Model of Inductor

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

$$\lvert Z \rvert = \sqrt{R_{dc}^2 + (2\pi f L)^2}$$

**Phase:**

$$\phi = \arctan\left(\frac{2\pi f L}{R_{dc}}\right)$$

### 7.4 Required Impedance Calculation

For target current $I_{coil}$ at supply voltage $V$:

$$\lvert Z_{coil} \rvert = \frac{V_{supply}}{I_{coil}}$$

|$V_{supply}$|$I_{coil}$|$\lvert Z \rvert$|
|---|---|---|
|9.0 V|40 mA|225 Ω|
|7.5 V|40 mA|187.5 Ω|
|6.0 V|40 mA|150 Ω|

**Design target: $\lvert Z \rvert \approx 190\ \Omega$** (for mid-life battery voltage)

### 7.5 Inductance Calculation

At 2 kHz, assuming $R_{dc} \ll X_L$:

$$\lvert Z \rvert \approx X_L = 2\pi f L$$

$$L = \frac{\lvert Z \rvert}{2\pi f} = \frac{190}{2\pi \times 2000} = \frac{190}{12566} = 15.1\ \text{mH}$$

**Target TX coil inductance: L = 15 mH**

### 7.6 Concentric Coil Design (Primary)

|Parameter|Value|Notes|
|---|---|---|
|Coil type|Concentric (3-coil)|TX + Bucking + RX|
|TX diameter|150 mm|Outer coil|
|TX turns|**130**||
|TX wire length|59 m||
|TX DC resistance|**4.7 Ω**|0.52 mm wire|
|TX inductance|**15 mH**||
|TX reactance @ 2kHz|188 Ω|$X_L = 2\pi fL$|
|TX impedance|**189 Ω**|$\sqrt{R^2 + X_L^2}$|
|TX Q factor|**40**|High Q with 0.52 mm wire|
|TX current @ 7.5V|**39.7 mA**|Within budget ✓|

**RX Coil:**

|Parameter|Value|
|---|---|
|Diameter|60 mm|
|Turns|180|
|Inductance|12 mH|
|R_dc|2.4 Ω|

**Bucking Coil:**

|Parameter|Value|
|---|---|
|Diameter|80 mm|
|Turns|15-25 (adjusted for null)|

### 7.7 Double-D Coil Design (Alternative)

|Parameter|TX Coil|RX Coil|
|---|---|---|
|Shape|D (80×150 mm)|D (80×150 mm)|
|Turns|100|120|
|Wire length|37 m|44 m|
|DC resistance|2.9 Ω|3.5 Ω|
|Inductance|11 mH|12 mH|
|Impedance @ 2kHz|**140 Ω**|—|
|Current @ 7.5V|**54 mA** ⚠️|—|

> [!warning] Double-D Issue
> Lower impedance means higher current (54 mA vs 40 mA budget).

**Solutions for Double-D:**

1. Add 47 Ω series resistor → limits to 40 mA
2. Reduce TX turns to ~75 → increases impedance
3. Accept 54 mA if optimizations free up headroom

### 7.8 Coil Configuration Comparison

|Parameter|Concentric|Double-D|
|---|---|---|
|TX inductance|15 mH|11 mH|
|TX impedance @ 2 kHz|189 Ω|140 Ω|
|TX current @ 7.5V|**40 mA ✓**|54 mA ⚠️|
|TX DC resistance|4.7 Ω|2.9 Ω|
|TX Q factor|40|48|
|Power in R_dc|7.5 mW|8.5 mW|
|Ground rejection|Moderate|Excellent|
|Pinpointing|Excellent|Good|
|Construction|Easier|Harder|

> [!tip] Recommendation
> Start with **Concentric** coil design — fits power budget perfectly.

---

## 8. TX Coil Driver Circuit

> **Detailed design:** See [[TX Driver and Tank Circuit Design]]

### 8.1 Amplifier Topology Selection

|Aspect|Class D (Switching)|Class AB (Linear)|
|---|---|---|
|**Efficiency**|90-99\%|50-70\%|
|**Heat dissipation**|Very low|High (~140 mW)|
|**Power to coil**|~340 mW|~220 mW|
|**Complexity**|Low-Medium|Low|

> [!success] Selected Topology
> **Class D (Switching)** — Higher efficiency critical for battery life.

### 8.2 Driver Options

|Option|Components|Efficiency|Drive Current|
|---|---|---|---|
|Simple NPN|2N2222A + 1N4148|94\%|2.9 mA|
|BJT Half-Bridge|2×NPN + 2×PNP|82\%|6 mA|
|**MOSFET Half-Bridge**|Si2301 + Si2302|**99.9\%**|**~0 mA**|

> [!tip] Recommended Driver
> **MOSFET Half-Bridge** — Saves 6 mA, virtually zero losses.

### 8.3 MOSFET Half-Bridge Design

```
                              V_bat (6-9V)
                                 │
                            ┌────┴────┐
                            │   Q1    │
                            │ Si2301  │  P-channel
    Pin 11 ────[100Ω]───────┤G   S   D├───┐
                            └─────────┘   │
                                          │
                                     ┌────┴────┐
                                     │ TX Coil │
                                     │  15 mH  │
                                     └────┬────┘
                                          │
                            ┌─────────────┘
                            │
                       ┌────┴────┐
                       │   Q2    │
    Pin 12 ────[100Ω]──┤G  D    S├───► GND
                       │ Si2302  │  N-channel
                       └─────────┘
```

### 8.4 Driver Efficiency Summary

|Parameter|Value|
|---|---|
|Input power (from battery)|$9V \times 40mA = 360$ mW|
|MOSFET conduction loss|0.24 mW|
|Switching loss|0.007 mW|
|Gate drive loss|0.1 mW|
|**Total driver loss**|**0.35 mW**|
|Power to coil|**359.65 mW**|
|**Driver efficiency**|**99.9\%**|

### 8.5 Complete Driver BOM

|Component|Value|Package|Notes|
|---|---|---|---|
|Q1|Si2301|SOT-23|P-channel high-side|
|Q2|Si2302|SOT-23|N-channel low-side|
|R_g1|100 Ω|0805|Gate resistor|
|R_g2|100 Ω|0805|Gate resistor|

**Optional for tank circuit:**

|Component|Value|Notes|
|---|---|---|
|C1|470 nF|Film, **250V** (high Q = high voltage!)|

---

## 9. Detection Range Analysis

### 9.1 Magnetic Dipole Model

A circular coil carrying current I can be modeled as a magnetic dipole:

$$m = N \times I \times A = N \times I \times \pi r^2$$

**For our TX coil (Concentric):**

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

> [!success] Assessment
> Detection at 50mm should be achievable with careful analog design

---

## 10. Complete Power Budget Summary

### 10.1 Option A: Maximum Power Mode (Recommended)

> **Philosophy:** Use all available power for maximum detection depth. Runtime exactly meets requirement.

```
┌──────────────────────────────────────────────────────────────────────┐
│               POWER BUDGET - MAXIMUM POWER MODE                      │
│                   (Concentric Coil, 0.52mm Wire)                     │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  BATTERY: Duracell MN1604 → ~350 mAh to 6V @ 120mA                  │
│  TARGET RUNTIME: 100 min (requirement)                               │
│  TX DUTY CYCLE: 100\% (continuous)                                    │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ELECTRONICS                                             80 mA      │
│  ├─ Arduino Mega 2560 Board                              62 mA      │
│  │   ├─ ATmega2560 (active, all peripherals)   25 mA               │
│  │   ├─ ATmega16U2 (USB interface)             12 mA               │
│  │   ├─ Power LED                               8 mA               │
│  │   ├─ NCP1117 regulator quiescent             5 mA               │
│  │   ├─ 3.3V subsystem                          2 mA               │
│  │   └─ PCB leakage, decoupling                10 mA               │
│  ├─ SSD1306 OLED Display                                 18 mA      │
│  └─ DSP Overhead                                        0.5 mA      │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TX COIL SYSTEM (100\% duty cycle)                        40 mA      │
│  ├─ TX Coil (L=15mH, Z=189Ω @ 7.5V)                      40 mA      │
│  ├─ Coil DC resistance loss (R=4.7Ω)                    7.5 mW      │
│  └─ Driver losses (MOSFET)                             <0.5 mW      │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TOTAL SYSTEM CURRENT                                   120 mA      │
│  EXPECTED RUNTIME                                      ~100 min     │
│                                                                      │
│  DETECTION PERFORMANCE                                  MAXIMUM     │
│  MAGNETIC FIELD STRENGTH                                  100\%      │
│                                                                      │
│  STATUS: ✅ MEETS REQUIREMENT - Maximum detection depth!            │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### 10.2 Option B: Conservative Mode (20\% Margin)

> **Philosophy:** Trade 20\% detection performance for 20\% runtime safety margin.

```
┌──────────────────────────────────────────────────────────────────────┐
│               POWER BUDGET - CONSERVATIVE MODE                       │
│                   (Concentric Coil, 0.52mm Wire)                     │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  BATTERY: Duracell MN1604 → ~400 mAh to 6V @ 100mA                  │
│  TARGET RUNTIME: 120 min (100 min + 20\% margin)                      │
│  TX DUTY CYCLE: 80\% (160ms ON / 40ms OFF)                           │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ELECTRONICS (with 80\% duty cycle)                       69 mA      │
│  ├─ ATmega2560 (80\% active, 20\% idle)                   21.1 mA     │
│  ├─ ATmega16U2 (always on)                               12 mA      │
│  ├─ Power LED                                             8 mA      │
│  ├─ Regulators + 3.3V                                     9 mA      │
│  ├─ PCB leakage                                           5 mA      │
│  └─ SSD1306 OLED (80\% active)                           14.4 mA     │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TX COIL SYSTEM (80\% duty cycle)                         32 mA      │
│  ├─ TX Coil (40mA × 80\%)                                 32 mA      │
│  └─ Driver losses                                       ~0 mA       │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  AVERAGE SYSTEM CURRENT                                 ~100 mA     │
│  EXPECTED RUNTIME                                      ~120 min     │
│                                                                      │
│  DETECTION PERFORMANCE                                     80\%      │
│  MAGNETIC FIELD STRENGTH                                   80\%      │
│  RUNTIME MARGIN                                        +20 min      │
│                                                                      │
│  STATUS: ✅ SAFE - Good balance of performance and margin           │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### 10.3 Mode Comparison

|Parameter|Maximum Power|Conservative|
|---|---|---|
|TX duty cycle|100\%|80\%|
|Average TX current|40 mA|32 mA|
|Total current|120 mA|100 mA|
|Runtime|~100 min|~120 min|
|**Magnetic field**|**100\%**|**80\%**|
|**Detection depth**|**100\%**|**~93\%**|
|Safety margin|0 min|+20 min|

> [!note] Detection Physics
> Detection depth scales as **cube root of power:** 80% field ≈ 93% depth

### 10.4 With LED Removed (Best Performance)

Removing the power LED frees 8 mA for either more margin or more TX power:

```
┌──────────────────────────────────────────────────────────────────────┐
│            POWER BUDGET - MAXIMUM POWER + LED REMOVED                │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Total current:                                         112 mA      │
│  Expected runtime:                                     ~110 min     │
│  Margin over requirement:                               +10 min     │
│                                                                      │
│  DETECTION PERFORMANCE                                  MAXIMUM     │
│                                                                      │
│  STATUS: ✅ BEST OPTION - Max power + small safety margin           │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### 10.5 Double-D Configuration Feasibility

|Mode|Concentric (40mA TX)|Double-D (54mA TX)|
|---|---|---|
|Maximum Power|120 mA → 100 min ✅|134 mA → 85 min ⚠️|
|Conservative (80\%)|100 mA → 120 min ✅|109 mA → 105 min ✅|
|With LED removed|112 mA → 110 min ✅|126 mA → 95 min ⚠️|

> **Double-D requires Conservative mode** to meet 100 min requirement, OR add 47Ω series resistor to limit TX current.

---

## 11. Power Management Strategy

### 11.0 Design Philosophy: Maximize Detection Power

> [!abstract] Design Goal
> Maximum TX power (= maximum detection depth) while meeting 100 min runtime requirement.
> We want to **use** the available power budget, not save it unnecessarily!

The requirement is 100 minutes runtime. Running for 200+ minutes means we're leaving detection performance on the table.

### 11.0.1 Design for 7.5V Supply Voltage

**Problem with designing for 9V:**
- Fresh battery: 9.4V under load
- Mid-life battery: 7.5V under load  
- End-of-life: 6.0V (cutoff)

If we design the TX driver for 9V operation:
- Performance is maximum only when battery is fresh
- Detection depth degrades continuously as battery discharges
- User experience is inconsistent

**Solution: Design all circuits for 7.5V**

By using 7.5V as the design voltage:

| Benefit | Explanation |
|---------|-------------|
| **Consistent performance** | Same TX current from fresh to near-depleted battery |
| **No compensation code** | No need for voltage sensing or adaptive drive |
| **Predictable behavior** | Detection depth stays constant throughout use |
| **Simpler design** | One operating point, not a variable range |

**Voltage availability analysis:**

| Battery State | Open Circuit | Under Load (100mA) | Time at State |
|---------------|--------------|-------------------|---------------|
| Fresh | 9.4V | ~9.2V | 0-20 min |
| Mid-life | 8.0V | ~7.5V | 20-70 min |
| Low | 7.0V | ~6.5V | 70-95 min |
| Cutoff | 6.5V | ~6.0V | 95-100 min |

**7.5V is available for ~80\% of the battery's useful life.** During the first 20 minutes (fresh battery), we simply have extra headroom. During the last 5-10 minutes, performance may drop slightly but remains acceptable until cutoff.

### 11.0.2 Power Redistribution Strategy

**Key Insight:** Detection depth depends on **peak magnetic field strength**, not average power consumption.

Instead of running continuously at moderate power, we can:
1. **Sleep during idle periods** → Reduce average system current
2. **Use the saved current budget** → Drive TX coil harder during active periods
3. **Result:** Higher peak magnetic field → Deeper detection

**Example calculation:**

| Mode | Duty Cycle | Average I | Peak TX I | Relative Field |
|------|------------|-----------|-----------|----------------|
| Continuous | 100\% | 120 mA | 40 mA | Baseline |
| Pulsed | 80\% | 100 mA | **50 mA** | **125\%** |
| Aggressive Pulsed | 50\% | 70 mA | **80 mA** | **200\%** |

By sleeping 20\% of the time, we save 20 mA average current. This headroom can be redirected to increase peak TX drive current from 40 mA to 50 mA during active periods.

**The math:**

At 100\% duty, 120 mA budget:
$$I_{TX,peak} = 120 - 80 = 40\ \text{mA}$$

At 80\% duty, same 120 mA average budget:
$$I_{avg} = I_{fixed} + I_{variable} \times 0.8 + I_{TX,peak} \times 0.8 = 120\ \text{mA}$$

Solving for increased peak TX current:
$$I_{TX,peak} = \frac{120 - 32 - (25 + 18) \times 0.8}{0.8} = \frac{120 - 32 - 34.4}{0.8} = \frac{53.6}{0.8} = 67\ \text{mA}$$

> **Result:** 80\% duty cycle allows **67 mA peak TX current** vs 40 mA continuous — a **68\% increase in peak field strength!**

**Trade-off considerations:**

| Factor | Continuous (100\%) | Pulsed (80\%) |
|--------|-------------------|--------------|
| Peak TX current | 40 mA | 67 mA |
| Peak magnetic field | Baseline | **+68\%** |
| Detection depth | Baseline | **~+19\%** (cube root) |
| Update rate | Continuous | 5 Hz |
| Code complexity | Simple | Moderate |
| Coil settling time | N/A | ~5 ms needed |

**Recommendation:** Use 80\% duty cycle with increased peak TX current for maximum detection depth while meeting runtime requirements.

### 11.1 Current Budget Analysis

**Target runtime:** 100 min (requirement) to 120 min (with 20\% margin)

From battery discharge curves:

- 100 min runtime → ~120 mA average current
- 120 min runtime → ~100 mA average current

**Fixed overhead (always on):**

|Component|Current|Notes|
|---|---|---|
|ATmega16U2|12 mA|USB chip, always powered|
|Power LED|8 mA|Can be removed|
|Regulators|7 mA|Quiescent current|
|PCB/misc|5 mA|Leakage, decoupling|
|**Fixed total**|**32 mA**|Cannot be duty-cycled|

**Variable overhead (scales with duty cycle X):**

|Component|Active|Sleep|Formula|
|---|---|---|---|
|ATmega2560|25 mA|5.5 mA|$5.5 + 19.5X$ mA|
|OLED|18 mA|~0 mA|$18X$ mA|
|TX Coil|40 mA|0 mA|$40X$ mA|

**Total average current:**

$$I_{avg} = 32 + 5.5 + (19.5 + 18 + 40) \times X = 37.5 + 77.5X\ \text{mA}$$

### 11.2 Optimal Duty Cycle Calculation

**For 120 min runtime (conservative, 20\% margin):**

Target $I_{avg} = 100$ mA:

$$100 = 37.5 + 77.5X$$ $$X = \frac{62.5}{77.5} = 80.6\%$$

**For 100 min runtime (maximum power):**

Target $I_{avg} = 120$ mA:

$$120 = 37.5 + 77.5X$$ $$X = \frac{82.5}{77.5} = 106\% \rightarrow 100\%$$

> **Result:** We can run at **100\% duty cycle** and still meet the 100 min requirement!

### 11.3 Operating Mode Options

|Mode|Duty Cycle|TX Average|Relative Field|Runtime|
|---|---|---|---|---|
|Maximum Power|**100\%**|**40 mA**|**100\%**|**~100 min**|
|Conservative|80\%|32 mA|80\%|~120 min|
|Battery Saver|50\%|20 mA|50\%|~160 min|
|Ultra Saver|25\%|10 mA|25\%|~210 min|

### 11.4 Recommended Configuration

#### Option A: Power Redistribution Mode (RECOMMENDED)

**Use when:** Maximum detection depth is priority. This is the optimal configuration.

**Strategy:** Use duty cycling to save average current, redirect savings to higher peak TX drive.

```
         ┌────────────────────────┐     ┌────────────────
    TX:  │    ON (67mA peak)      │     │   ON (67mA)
         ┘                        └─────┘
         ├────────160ms───────────┤─40ms┤
         
              Sample + DFT          Sleep
         
    vs Continuous mode: ════════════════════════════════
                          ON (40mA continuous)
```

|Parameter|Value|
|---|---|
|Design voltage|**7.5V** (consistent across battery life)|
|TX duty cycle|80\%|
|Active period|160 ms (320 TX cycles)|
|Sleep period|40 ms|
|Update rate|5 Hz|
|**Peak TX current**|**67 mA** (vs 40 mA continuous)|
|Average TX current|53.6 mA|
|Total system current|~120 mA average|
|Expected runtime|~100 min|
|**Detection performance**|**+68\% field strength, +19\% depth vs continuous**|

**Why this beats continuous operation:**

| Metric | Continuous (100\%) | Pulsed (80\%) | Improvement |
|--------|-------------------|--------------|-------------|
| Peak TX current | 40 mA | 67 mA | **+68\%** |
| Peak magnetic field | Baseline | +68\% | **+68\%** |
| Detection depth | Baseline | +19\% | **+19\%** |
| Runtime | 100 min | 100 min | Same |

**Implementation notes:**
- TX driver must be designed for 67 mA at 7.5V
- Coil needs ~5 ms settling time after TX enable
- MCU enters IDLE sleep during off periods
- OLED can remain on (small current vs TX savings)

#### Option B: Simple Continuous Mode

**Use when:** Simplicity is priority over maximum detection depth.

```
    TX: ████████████████████████████████████████████████
        ├──────────────── Continuous ─────────────────►
        
        100\% duty cycle @ 40mA = Baseline magnetic field
```

|Parameter|Value|
|---|---|
|Design voltage|7.5V|
|TX duty cycle|100\%|
|TX current|40 mA (continuous)|
|Total system current|120 mA|
|Expected runtime|~100 min|
|Detection performance|**Baseline**|

**Simpler code, but 19\% less detection depth than Option A.**

#### Option C: Conservative Mode (Extra Runtime Margin)

**Use when:** Want 20\% runtime safety margin, don't need maximum detection depth.

```
         ┌────────────────────────┐     ┌────────────────
    TX:  │     ON (40mA std)      │     │   ON (40mA)
         ┘                        └─────┘
         ├────────160ms───────────┤─40ms┤
         
              Sample + DFT          Sleep
```

|Parameter|Value|
|---|---|
|Design voltage|7.5V|
|TX duty cycle|80\%|
|Active period|160 ms (320 TX cycles)|
|Sleep period|40 ms|
|Update rate|5 Hz|
|Peak TX current|40 mA (standard)|
|Average TX current|32 mA|
|Total system current|~100 mA|
|Expected runtime|**~120 min (+20\% margin)**|
|Detection performance|**80\% of baseline**|

**Use this if:** Battery quality is uncertain, or you want extra runtime margin. Does NOT redirect saved power to TX coil.

### 11.5 Mode Comparison Summary

|Mode|Duty|Peak TX|Avg Current|Runtime|Detection Depth|
|---|---|---|---|---|---|
|**A: Power Redistribution**|80\%|**67 mA**|120 mA|100 min|**+19\% vs baseline**|
|B: Simple Continuous|100\%|40 mA|120 mA|100 min|Baseline|
|C: Conservative|80\%|40 mA|100 mA|120 min|80\% of baseline|

> **Recommendation:** Use **Option A (Power Redistribution)** for maximum detection performance at 7.5V design voltage.

### 11.6 Physical Constraints (Why Updates > 5 Hz Don't Help)

|Parameter|Value|Notes|
|---|---|---|
|Typical sweep speed|0.5 m/s|Walking pace|
|Coil diameter|150 mm||
|Target traverse time|300 ms|Time for object to cross coil|
|Nyquist update rate|3.3 Hz|Minimum to not miss targets|
|Chosen update rate|5 Hz|50\% margin|

At 5 Hz updates with 0.5 m/s sweep:

- Sample spacing: 100 mm
- Guaranteed overlap with 150 mm coil ✓

### 11.7 Implementation: Power Redistribution Mode (Option A - RECOMMENDED)

```c
#define DESIGN_VOLTAGE     7.5      // Design for mid-life battery voltage
#define ACTIVE_PERIOD_MS   160      // 320 TX cycles at 2kHz
#define SLEEP_PERIOD_MS    40       // Brief sleep
#define UPDATE_RATE_HZ     5        // 200ms total period
#define TX_PEAK_CURRENT_MA 67       // Increased from 40mA baseline

void measurement_cycle(void) {
    // Turn on TX driver (configured for 67mA at 7.5V)
    enable_tx_driver();
    
    // Wait for coil to stabilize (~5ms)
    _delay_ms(5);
    
    // Sample for 155ms (310 cycles at 2kHz)
    start_adc_sampling();
    _delay_ms(ACTIVE_PERIOD_MS - 5);
    stop_adc_sampling();
    
    // Turn off TX driver
    disable_tx_driver();
    
    // Compute DFT and update display (~10ms)
    compute_dft();
    update_display();
    
    // Enter idle sleep for remaining time
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_for_ms(SLEEP_PERIOD_MS - 10);  // Account for processing
}
```

**TX Driver Design for 67mA at 7.5V:**

The TX coil impedance at 2kHz with 15mH inductance and 4.7Ω DC resistance:
$$Z_{coil} = \sqrt{R_{dc}^2 + (2\pi f L)^2} = \sqrt{4.7^2 + (2\pi \times 2000 \times 0.015)^2} = \sqrt{22 + 35400} = 188\ \Omega$$

For 67mA peak current:
$$V_{required} = I \times Z = 0.067 \times 188 = 12.6\ V_{peak}$$

This requires a voltage boost or H-bridge driver to achieve higher peak voltage than the 7.5V supply. Options:
1. **H-bridge driver** - doubles effective voltage swing (±7.5V = 15V peak-to-peak)
2. **Resonant tank circuit** - Q factor multiplies voltage at resonance
3. **Boost converter** - increase supply voltage to TX driver

### 11.8 Implementation: Simple Continuous Mode (Option B)

```c
#define DESIGN_VOLTAGE     7.5      // Design for mid-life battery voltage
#define TX_CURRENT_MA      40       // Standard continuous current

// No duty cycling - continuous operation at 7.5V design point
void main_loop(void) {
    // TX runs continuously (Timer1 hardware PWM)
    // ADC sampling synchronized to TX (Timer1 triggered)
    
    while (1) {
        if (samples_ready) {
            compute_dft();
            update_display();  // Throttle to ~5 Hz
            samples_ready = 0;
        }
    }
}
```

**Simpler than Option A, but 19\% less detection depth.**

---

### 11.9 Additional Optimizations (If Margin Needed)

**Priority 1: Remove Power LED** → saves 8 mA

- Cut PWR LED jumper or desolder LED
- Zero functional impact
- Frees 8 mA for more TX power or longer runtime

**With LED removed (Power Redistribution mode):**

|Configuration|Peak TX|Avg Current|Runtime|Depth Improvement|
|---|---|---|---|---|
|Standard|67 mA|120 mA|100 min|+19\%|
|LED removed|**75 mA**|120 mA|100 min|**+28\%**|

**Priority 2: Use bare ATmega2560** (future optimization)

- Remove Arduino board, use bare chip
- Eliminates ATmega16U2 (12 mA) and other overhead
- Potential savings: 20-30 mA
- Allows even higher TX current or much longer runtime

### 11.10 Impact of 0.52mm Wire Choice

|Parameter|0.3mm Wire|0.52mm Wire|Improvement|
|---|---|---|---|
|DC resistance|14.6 Ω|4.7 Ω|**68\% lower**|
|Q factor|12.9|40|**3× higher**|
|Power in R_dc|23 mW|7.5 mW|**67\% less heat**|
|Coil bandwidth|155 Hz|50 Hz|Narrower (more selective)|

Lower DC resistance means more power reaches the magnetic field instead of being wasted as heat.

### 11.11 Why MOSFET H-Bridge Driver is Essential

For the Power Redistribution strategy, the MOSFET H-bridge is critical:

|Parameter|Simple NPN|MOSFET H-Bridge|
|---|---|---|
|Drive current|2.9 mA|~0 mA|
|Driver loss|20 mW|0.35 mW|
|Efficiency|94\%|99.9\%|
|**Voltage swing**|0 to V+|**-V+ to +V+**|
|**Effective peak voltage**|7.5V|**15V (doubled)**|

**MOSFET H-bridge doubles the effective voltage swing**, enabling the higher peak currents needed for the power redistribution strategy. With an H-bridge, you get ±7.5V swing (15V peak-to-peak) from a 7.5V supply.

### 11.12 Summary: Recommended Operating Modes

**Design Voltage: 7.5V** (consistent performance throughout battery life)

|Coil Type|Mode|Peak TX|Avg Current|Runtime|Detection Depth|
|---|---|---|---|---|---|
|**Concentric**|**Power Redistribution**|**67 mA**|**120 mA**|**100 min**|**+19\% vs baseline ✅**|
|Concentric|Simple Continuous|40 mA|120 mA|100 min|Baseline|
|Concentric|Conservative|40 mA|100 mA|120 min|80\% of baseline|
|Concentric|Redistribution + no LED|75 mA|120 mA|100 min|**+28\% vs baseline**|
|Double-D|Conservative|43 mA|109 mA|105 min|80\% of baseline|

> [!tip] Final Recommendation
> - **Use Power Redistribution Mode** with 7.5V design voltage and MOSFET H-bridge driver
> - **Concentric coil:** 67 mA peak TX current → +19% detection depth vs continuous
> - **Remove Power LED** for additional 8 mA headroom → 75 mA peak → +28% depth
> - **Double-D:** Use Conservative mode with standard 40 mA to meet runtime

---

## 12. Verification Test Plan

### 12.1 Component Tests

|Test|Setup|Pass Criteria|
|---|---|---|
|Arduino + OLED|AD3 @ 9V via shunt|< 85 mA|
|TX driver alone|Scope + DMM|35-45 mA|
|Full electronics|Shunt + battery|< 85 mA|
|Full system (Concentric)|Shunt + battery|115-125 mA|
|Full system (Double-D)|Shunt + battery|130-140 mA|

### 12.2 Coil Verification

|Test|Method|Pass Criteria|
|---|---|---|
|TX inductance|LCR meter @ 1kHz|14-16 mH|
|TX resistance|Multimeter|4-6 Ω|
|RX inductance|LCR meter @ 1kHz|10-14 mH|
|Induction balance|Oscilloscope|< 1 mV @ RX|

### 12.3 Runtime Test Protocol

**Test both operating modes:**

#### Maximum Power Mode (100\% duty cycle)

|Time (min)|Target I (mA)|$V_{bat}$ (V)|Status|
|---|---|---|---|
|0|120|>9.0|Start|
|25|120|>8.0||
|50|120|>7.5||
|75|120|>7.0||
|**100**|120|**>6.0**|**PASS?**|

#### Conservative Mode (80\% duty cycle)

|Time (min)|Target I (mA)|$V_{bat}$ (V)|Status|
|---|---|---|---|
|0|100|>9.0|Start|
|30|100|>8.2||
|60|100|>7.6||
|90|100|>7.0||
|**120**|100|**>6.0**|**PASS?**|

### 12.4 Detection Performance Test

Verify detection depth correlates with TX power:

|Mode|TX Current|Expected Depth|Measured Depth|
|---|---|---|---|
|Maximum (100\%)|40 mA|50 mm baseline||
|Conservative (80\%)|32 mA|~47 mm (93\%)||
|Battery Saver (50\%)|20 mA|~40 mm (80\%)||

> Test target: Ø30mm × 50mm iron cylinder

---

## 13. References

1. [[Literature/duracell_9volt.pdf|Duracell 9V Datasheet]]
2. [[kravspecifikation.pdf|Kravspecifikation]]
3. [[Coil Basics.pdf|Coil Design Fundamentals]]
4. [[Search Coil Design]]
5. [[TX Driver and Tank Circuit Design]]
6. ATmega2560 Datasheet, Microchip Technology
7. 2N2222A Datasheet, ON Semiconductor
8. Si2301/Si2302 Datasheet, Vishay
9. SSD1306 Datasheet, Solomon Systech

---

#power #calculations #coil-design #driver #measurements #requirements