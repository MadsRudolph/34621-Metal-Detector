# Coil Winder - Bill of Materials

> [!abstract] Document Purpose
> Complete parts list for the simplified coil winder.
> **Priority: DTU Component Shop first, then your kit for what's not available.**

---

## 1. Cost Summary

| Source | Cost |
|--------|------|
| DTU Component Shop | €0 |
| Your Kit (only what DTU doesn't have) | €0 |
| Hardware Store | ~€10 |
| 3D Printing | ~€15 |
| **Total** | **~€25** |

---

## 2. From DTU Component Shop (Free) - Primary Source

| Component | DTU Part | Qty | Use |
|-----------|----------|-----|-----|
| ITR8307 | ITR8307 | 1 | Turn counting optical sensor |
| 330Ω Resistor | 330R (E96) | 1 | IR LED current limit |
| 10kΩ Resistor | 10K0 (E96) | 1 | Sensor pull-up |
| 100nF Capacitor | 100n Ceramic | 2 | Noise filtering |
| 100µF Capacitor | 100µF Electrolytic | 1 | Power filtering |
| Pushbuttons | Pushbutton / Tryk knap til bredbord | 3 | Reset, Preset, Start |
| Pin Headers | Pin header strips | 1 | Connections |

---

## 3. From Your Kit (Only What DTU Doesn't Have)

| Component | Why from kit |
|-----------|--------------|
| Arduino UNO | No microcontrollers at DTU |
| Servo Motor | No servos at DTU |
| Piezo Buzzer | No buzzers at DTU (optional - can skip) |
| Jumper Wires | Convenient, already have |

---

## 4. From Your Existing Parts

| Component | Use |
|-----------|-----|
| OLED Display 128x64 | Turn count, progress display |

---

## 5. Hardware Store (~€10)

| Component | Qty | Est. Cost | Notes |
|-----------|-----|-----------|-------|
| M8 Threaded Rod | 30cm | €2 | Spindle shaft |
| 608ZZ Bearings | 2 | €3 | Skateboard bearings |
| M8 Nuts | 6 | €1 | |
| M8 Washers | 4 | €0.50 | |
| M3×10 Bolts | 10 | €1.50 | Frame assembly |
| M3 Nuts | 10 | €0.50 | |
| Small spring | 1 | €1 | Wire tensioner (optional) |

---

## 6. 3D Printing (~€15 filament, DTU Lab)

| Part | Filament | Notes |
|------|----------|-------|
| Base plate | ~150g | Main frame |
| Bearing mounts (2) | ~60g | Hold spindle |
| Coil adapters (3) | ~80g | TX 200mm, RX 80mm, Buck 120mm |
| Hand crank | ~20g | |
| Encoder wheel | ~5g | Single slot, triggers ITR8307 |
| Servo mount + linkage | ~25g | |
| Wire guide carriage | ~15g | Slides on rail |
| **Total** | **~355g** | ~15-18 hours print time |

---

## 7. Complete Shopping Checklist

### DTU Component Shop (Get These First)
- [ ] ITR8307 optical sensor ×1
- [ ] 330Ω resistor (330R) ×1
- [ ] 10kΩ resistor (10K0) ×1
- [ ] 100nF ceramic capacitor ×2
- [ ] 100µF electrolytic capacitor ×1
- [ ] Pushbuttons (breadboard type) ×3
- [ ] Pin header strip ×1

### From Your Arduino Kit
- [ ] Arduino UNO
- [ ] Servo motor
- [ ] Piezo buzzer (optional)
- [ ] Jumper wires

### From Your Parts
- [ ] OLED display

### Hardware Store (~€10)
- [ ] M8 threaded rod 30cm
- [ ] 608ZZ bearings ×2
- [ ] M8 nuts ×6
- [ ] M8 washers ×4
- [ ] M3×10 bolts ×10
- [ ] M3 nuts ×10

### 3D Print at DTU Lab (~€15)
- [ ] Base plate
- [ ] Bearing mounts ×2
- [ ] Coil adapters ×3 (200mm, 120mm, 80mm)
- [ ] Hand crank
- [ ] Encoder wheel
- [ ] Servo mount + linkage
- [ ] Wire guide carriage

---

## 8. Wiring Diagram

```
                      ARDUINO UNO
                     ┌───────────┐
                     │           │
    ITR8307 ────────►│ D2        │
                     │           │
    SERVO ──────────►│ D9        │
                     │           │
    BTN_RESET ──────►│ D3        │
    BTN_PRESET ─────►│ D4        │
    BTN_START ──────►│ D5        │
                     │           │
    BUZZER ─────────►│ D10       │  (optional)
                     │           │
    OLED SDA ───────►│ A4        │
    OLED SCL ───────►│ A5        │
                     │           │
                     │ 5V ───────┼──► Power for all components
                     │ GND ──────┼──► Common ground
                     └───────────┘
```

### ITR8307 Wiring (DTU Component)

```
    ITR8307 (Optical Slot Sensor)
    ┌─────────────┐
    │  ┌─┐   ┌─┐  │
    │  │E│   │D│  │     E = Emitter (IR LED)
    │  │ │   │ │  │     D = Detector (Phototransistor)
    │  └┬┘   └┬┘  │
    └───┼─────┼───┘
        │     │
       1 2   3 4

    Pin 1 (Anode)    ────► 330Ω (DTU) ────► 5V
    Pin 2 (Cathode)  ────► GND
    Pin 3 (Collector)────► D2 + 10kΩ (DTU) to 5V
    Pin 4 (Emitter)  ────► GND
```

### Button Wiring (DTU Components)

```
    Each pushbutton (from DTU):

    Arduino Pin (D3/D4/D5) ────┬──── [Button] ──── GND
                               │
                          (internal pull-up)
```

### Servo Wiring (From Kit)

```
    Servo Wire     Arduino
    ──────────     ───────
    Brown/Black    GND
    Red            5V
    Orange/Yellow  D9

    Add 100µF capacitor (DTU) between 5V and GND near servo
```

### OLED Wiring (Your Part)

```
    OLED Pin    Arduino
    ────────    ───────
    VCC         5V
    GND         GND
    SDA         A4
    SCL         A5
```

---

## 9. Assembly Overview

### Step 1: Get Components
1. Visit DTU component shop - get all items from Section 2
2. Collect Arduino, servo, buzzer from your kit
3. Get your OLED display
4. Buy hardware store items

### Step 2: Build Frame
```
    ┌─────────────────────────────────────────┐
    │                                         │
    │  [Bearing]────[Spindle]────[Bearing]    │
    │      │                          │       │
    │  [Mount]                    [Mount]     │
    │      │                          │       │
    │  ════╧══════════════════════════╧════   │  Base Plate
    │              │                          │
    │         [Hand Crank]                    │
    │                                         │
    │    [Servo]──[Linkage]──[Wire Guide]     │
    │                                         │
    │    [ITR8307]◄───[Encoder Wheel]         │
    │              (on spindle)               │
    └─────────────────────────────────────────┘
```

### Step 3: Wire Electronics
1. Connect ITR8307 with 330Ω and 10kΩ (both from DTU)
2. Connect servo to D9
3. Connect 3 pushbuttons (DTU) to D3, D4, D5
4. Connect OLED to A4, A5
5. Add 100nF caps (DTU) for noise filtering
6. Add 100µF cap (DTU) on servo power
7. Optional: Connect buzzer (kit) to D10

### Step 4: Upload Firmware
```bash
# PlatformIO (embedded C):
cd coil_winder
pio run -t upload
```

### Step 5: Test & Calibrate
1. Open Serial Monitor (115200 baud)
2. Send `+` to simulate turns - verify servo moves
3. Send `<` and `>` to jog traverse
4. Adjust `SERVO_CENTER` and `SERVO_RANGE` in code if needed

---

## 10. Servo Linkage Mechanism

```
    Side View:

              Servo Horn (15mm)
                   │
    [SERVO]────────┼────────┐
                   │        │
                   ▼        │ Push Rod
              ┌────────┐    │
              │        │◄───┘
              │ Guide  │
              │Carriage│ ═══════ Slides on rail
              └────┬───┘
                   │
              Wire Guide
                   │
                   ▼
              [Coil Form]

    Servo rotation: ~50° = ~18mm linear travel
```

---

## 11. Encoder Wheel Design

```
    ┌─────────────────┐
    │                 │
    │    ┌───────┐    │
    │    │       │    │    Single slot
    │    │  ███  │    │    (blocks IR beam once per revolution)
    │    │       │    │
    │    └───────┘    │
    │                 │
    │        ●        │    Center hole (fits M8 spindle)
    │                 │
    └─────────────────┘

    Diameter: ~40mm
    Slot width: ~5mm
    Slot depth: ~10mm (must fully block ITR8307 gap)
```

---

## 12. Coil Specifications Reference

| Coil | Diameter | Turns | Layers | Width | Wire |
|------|----------|-------|--------|-------|------|
| TX | 200mm | 68 | 2 | 18mm | 0.52mm |
| RX | 80mm | 170 | 4 | 14mm | 0.32mm |
| Bucking | 120mm | 35 | 1 | 18mm | 0.52mm |

---

## 13. Troubleshooting

| Problem | Solution |
|---------|----------|
| Display blank | Check I2C wiring, try address 0x3D instead of 0x3C |
| No turn counting | Check ITR8307 wiring, verify encoder wheel blocks beam |
| Servo jitters | Add 100µF capacitor (DTU) on servo power |
| Servo doesn't reach full width | Increase SERVO_RANGE in code |
| Traverse reversed | Swap SERVO_MIN and SERVO_MAX in code |
| Buttons not working | Check wiring to GND |

---

## 14. Component Sources Summary

| Component | Source | Reason |
|-----------|--------|--------|
| ITR8307 | **DTU** | Best sensor for turn counting |
| 330Ω resistor | **DTU** | For ITR8307 LED |
| 10kΩ resistor | **DTU** | For ITR8307 pull-up |
| 100nF capacitors | **DTU** | Noise filtering |
| 100µF capacitor | **DTU** | Power filtering |
| Pushbuttons | **DTU** | User input |
| Pin headers | **DTU** | Connections |
| Arduino UNO | Kit | Not available at DTU |
| Servo motor | Kit | Not available at DTU |
| Piezo buzzer | Kit | Not available at DTU (optional) |
| OLED display | Yours | Already have |
| Hardware | Store | Mechanical parts |

---

*Coil Winder for DTU 34621 Metal Detector Project*
*Total cost: ~€25*
