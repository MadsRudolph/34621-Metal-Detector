
> **Requirement:** Metal detector must run for 100 minutes on a 9V battery (6LR61) with remaining voltage >6V after the 100 minutes.
> 
> See: [[kravspecifikation.pdf|Kravspecifikation]] requirements 4 & 5

---

## 📊 Battery Specifications (Duracell MN1604)

> 📚 **Datasheet:** [[Literature/duracell_9volt.pdf|Duracell 9V Datasheet]]

|Parameter|Value|
|---|---|
|Nominal Voltage|9V|
|Operating Voltage|9.6V - 4.8V|
|Internal Impedance|1.7Ω @ 1kHz|
|Capacity (to 6V)|~400-500 mAh (load dependent)|

### Discharge Curves (from datasheet)

|Constant Current|Runtime to 6V|
|---|---|
|10 mA|>400 hours|
|50 mA|~10 hours (600 min)|
|100 mA|~3.5 hours (210 min)|
|150 mA|~2 hours (120 min)|

---

## 🎯 Power Budget Calculation

### Requirement

- Runtime: **100 minutes minimum**
- End voltage: **>6V**
- Safety margin: **1.5×** recommended

### Maximum Allowable Current

From the discharge curves, to guarantee 100 min with margin:

$$I_{max} = 120\ \text{mA (conservative)}$$

With 1.5× safety margin for temperature and battery variation:

$$I_{target} = \frac{120\ \text{mA}}{1.5} = 80\ \text{mA}$$

> ⚠️ **Target total system current: 80-120 mA**

---

## 🔬 Measurement Procedure

### Equipment Needed

- [ ] Digital multimeter (mA range, 200mA or higher)
- [ ] 9V battery (fresh Duracell MN1604)
- [ ] Arduino Mega 2560
- [ ] SSD1306 OLED display
- [ ] MCP3208 ADC
- [ ] Jumper wires
- [ ] Optional: 1Ω precision resistor + oscilloscope

### Method 1: Multimeter in Series (Recommended)

```
         ┌─────────────────┐
9V (+) ──┤ Multimeter (mA) ├──→ Arduino Vin
         └─────────────────┘
9V (-) ─────────────────────────→ Arduino GND
```

**Steps:**

1. [ ] Set multimeter to **DC mA** mode (200mA range)
2. [ ] Connect multimeter **in series** between battery (+) and Arduino Vin
3. [ ] Connect battery (-) directly to Arduino GND
4. [ ] Power on and wait 10 seconds for stabilization
5. [ ] Record current reading

### Method 2: Shunt Resistor (For dynamic measurement)

```
         ┌──────────┐
9V (+) ──┤ 1Ω 1%   ├──→ Arduino Vin
         └────┬─────┘
              │ Measure voltage
              │ across resistor
         ┌────┴─────┐
         │ Scope/DMM │
         └──────────┘
9V (-) ─────────────────→ Arduino GND
```

**Calculation:** $I = V_{shunt} / 1Ω$ (1mV = 1mA)

---

## 📝 Measurement Log

### Test Conditions

- Date: _______________
- Battery: Duracell MN1604, Fresh: [ ] Yes [ ] No
- Ambient temp: _______°C
- Battery voltage (no load): _______V

### Component Measurements

|#|Configuration|Current (mA)|Notes|
|---|---|---|---|
|1|Arduino Mega only (idle)||No code running|
|2|Arduino Mega (code running)||Main loop active|
|3|+ SSD1306 OLED||Display showing data|
|4|+ MCP3208 ADC||SPI active @ 8kHz|
|5|+ TX Coil driver (no coil)||Driver circuit only|
|6|+ TX Coil connected||Full system|

### Calculated Budget

```
Total electronics (row 4):     _______ mA
Maximum budget:                   120 mA
─────────────────────────────────────────
Available for TX coil:         _______ mA
```

---

## 📈 Expected Values

|Component|Typical Current|Your Measurement|
|---|---|---|
|Arduino Mega (5V reg, running)|50-80 mA||
|SSD1306 OLED (full brightness)|15-25 mA||
|MCP3208 ADC|0.4-0.5 mA||
|**Subtotal (electronics)**|**70-105 mA**||
|**Remaining for TX coil**|**15-50 mA**||

---

## 🔌 TX Coil Power Calculation

Once you know the available current for the coil:

### Power Available

$$P_{coil} = V_{supply} \times I_{available}$$

Example with 30mA available at 9V: $$P_{coil} = 9V \times 30mA = 270mW$$

### Coil Current vs Detection Range

|Coil Current|Power @ 9V|Expected Detection|
|---|---|---|
|10 mA|90 mW|~30mm|
|20 mA|180 mW|~40mm|
|30 mA|270 mW|~50mm ✓|
|50 mA|450 mW|~60mm|

> **Requirement:** Detect iron (15mm radius, 50mm length) at 50mm depth

---

## ✅ Verification Test

After building the complete system:

1. [ ] Charge/replace 9V battery (measure: ______V)
2. [ ] Power on metal detector
3. [ ] Run for **100 minutes** continuously
4. [ ] Measure battery voltage after test: ______V
5. [ ] **Pass criteria:** Final voltage > 6.0V

### Test Log

|Time (min)|Battery Voltage|Notes|
|---|---|---|
|0||Start|
|25|||
|50|||
|75|||
|100||End - must be >6V|

---

## 🔧 If Over Budget

Options to reduce power consumption:

### Software

- [ ] Reduce OLED refresh rate (currently 12Hz)
- [ ] Dim OLED display
- [ ] Sleep MCU between measurements
- [ ] Reduce sampling rate (affects performance)

### Hardware

- [ ] Use Arduino Pro Mini (3.3V, 8MHz) — ~5mA
- [ ] Use ATmega328P standalone (no USB chip) — ~15mA
- [ ] Lower TX coil current (trade detection range)
- [ ] Use more efficient voltage regulator

### Measured Savings

|Optimization|Current Saved|
|---|---|
|OLED 50% brightness|~10 mA|
|Remove USB chip (standalone)|~20 mA|
|8MHz instead of 16MHz|~10 mA|

---

## 📎 Related Notes

- [[Theory References#Electronics|Electronics Theory]]
- [[KiCad/README|PCB Design]]
- [[LTspice/README|Circuit Simulations]]

---

#power #measurement #battery #requirements