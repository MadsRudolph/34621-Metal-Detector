# 🔬 Current Measurement with Analog Discovery 3

> Measure Arduino Mega current draw using a shunt resistor and AD3

---

## 📦 Equipment Needed

- [ ] Analog Discovery 3
- [ ] 1Ω precision resistor (1% or better, ≥0.5W)
- [ ] 9V battery (fresh Duracell MN1604)
- [ ] Arduino Mega 2560
- [ ] SSD1306 OLED display
- [ ] Jumper wires
- [ ] Breadboard

---

## 🔌 Wiring Diagram

```
                    ┌─────────┐
9V Battery (+) ─────┤  1Ω     ├─────┬────→ Arduino Vin
                    └─────────┘     │
                          │         │
           Scope 1+ ──────┘         │
           (orange)                 │
                                    │
           Scope 1- ────────────────┘
           (orange/white)
           
9V Battery (-) ─────────────────────────→ Arduino GND
                          │
           Any black ─────┘ (optional extra ground)
```

### Connection Summary

| Wire | From | To |
|------|------|----|
| Red | 9V (+) | Resistor input |
| Resistor output | - | Arduino Vin |
| Scope 1+ (orange) | - | Resistor output (Arduino Vin side) |
| Scope 1- (orange/white) | - | Resistor input (Battery side) |
| Black | 9V (-) | Arduino GND |

### Why This Configuration?

We measure the voltage **across** the resistor:
- Scope 1+ on the LOW side (Arduino Vin)
- Scope 1- on the HIGH side (Battery +)

This gives a **negative** voltage reading (since current flows from + to -), but the magnitude tells us current:

$$I = \frac{V_{shunt}}{R} = \frac{V_{shunt}}{1Ω}$$

**1 mV = 1 mA** (with 1Ω resistor)

---

## ⚙️ WaveForms Settings

### Scope Configuration

| Setting | Value | Notes |
|---------|-------|-------|
| **Channel 1** | Enabled | |
| **Range** | 100 mV/div | For 0-500mA measurement |
| **Offset** | 50 mV | Center the trace |
| **Coupling** | DC | |
| **Time Base** | 200 ms/div | See dynamics |
| **Trigger** | Auto | Free running |

### For Steady-State Measurement

| Setting | Value |
|---------|-------|
| Mode | Repeated |
| Time Base | 1 s/div |
| Persistence | Infinite |

### Measurements to Add

1. Click **Measurements** tab
2. Add these measurements for Channel 1:
   - **Average** → This is your current in mV (= mA)
   - **Min** 
   - **Max**
   - **RMS**

---

## 📊 Reading the Results

### Voltage to Current Conversion

| Scope Reading | Current |
|---------------|---------|
| -50 mV | 50 mA |
| -80 mV | 80 mA |
| -100 mV | 100 mA |
| -120 mV | 120 mA |

> ⚠️ Reading will be **negative** due to measurement polarity. Use absolute value.

### Expected Readings

| Test Condition | Expected Voltage | Expected Current |
|----------------|------------------|------------------|
| Mega idle | -50 to -60 mV | 50-60 mA |
| Mega + code running | -70 to -80 mV | 70-80 mA |
| + OLED active | -90 to -105 mV | 90-105 mA |
| + All peripherals | -100 to -120 mV | 100-120 mA |

---

## 🧪 Test Procedure

### 1. Flash the Test Code

```bash
# Edit PORT in Makefile to match your Arduino
make clean
make
make upload
```

Or use Arduino IDE:
1. Copy `power_test.c` content
2. Create new sketch, paste and rename to `.ino`
3. Upload to Arduino Mega

### 2. Disconnect USB

After uploading:
1. Disconnect USB cable
2. Connect 9V battery through shunt resistor
3. Connect Analog Discovery probes

> ⚠️ **Important:** USB must be disconnected! USB power affects measurement.

### 3. Start Measurement

1. Open WaveForms → Scope
2. Apply settings from table above
3. Click **Run**
4. Wait 10-30 seconds for readings to stabilize
5. Record Average measurement

### 4. Measurement Sequence

Measure each configuration separately:

| Step | Configuration | Disconnect | Reconnect | Reading (mV) | Current (mA) |
|------|---------------|------------|-----------|--------------|--------------|
| 1 | Mega only | - | - | | |
| 2 | + OLED | OLED | OLED | | |
| 3 | Full system | - | - | | |

---

## 📈 Capturing Data

### Export for Report

1. Run scope for 10+ seconds
2. Click **File → Export**
3. Save as CSV for analysis
4. Screenshot the scope view

### Long-term Logging

To verify 100-minute runtime:

1. **Scope** → **Logging** tab
2. Set sample interval: 1 second
3. Set duration: 110 minutes
4. Log Channel 1 Average
5. Export to CSV
6. Plot voltage vs time in Excel/MATLAB

---

## 🧮 Calculate Power Budget

After measurements:

```
Measured current:        _______ mA
Maximum allowed:            120 mA
────────────────────────────────────
Remaining for TX coil:   _______ mA

Coil power budget:
P = V × I = 9V × ___mA = ___ mW
```

---

## ⚠️ Troubleshooting

| Problem | Solution |
|---------|----------|
| Reading is 0 | Check shunt resistor connections |
| Reading is very noisy | Add 100nF capacitor across shunt |
| Reading is positive | Swap Scope 1+ and 1- |
| OLED not working | Check I2C address (0x3C vs 0x3D) |
| No serial output | Check baud rate (9600) |

---

## 🔗 Related Notes

- [[Power Budget Analysis]]
- [[Code/src/power_test.c|Power Test Code]]
- [[Literature/duracell_9volt.pdf|Battery Datasheet]]

---

#measurement #power #analog-discovery #lab
