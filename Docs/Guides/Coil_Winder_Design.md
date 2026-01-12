# Coil Winder Design - VLF Metal Detector

> [!abstract] Document Purpose
> Complete design documentation for a precision coil winder with **servo-driven traverse** to wind TX, RX, and Bucking coils for the DTU 34621 Metal Detector project.
>
> **Target:** Wind three coils with ±1 turn accuracy, uniform layer packing, and automatic back-and-forth traverse
> **Implementation:** Embedded C firmware, minimal cost using available components

---

## 1. Coil Specifications Summary

### 1.1 Coil Requirements from Project

| Parameter | TX Coil | RX Coil | Bucking Coil |
|-----------|---------|---------|--------------|
| **Form Diameter** | 200 mm | 80 mm | 120 mm |
| **Wire Gauge** | 0.52 mm (AWG24) | 0.32 mm (AWG28) | 0.52 mm (AWG24) |
| **Total Turns** | 68 | ~170 | ~35 |
| **Turns per Layer** | 34 | ~42 | ~35 |
| **Layers** | 2 | 4 | 1-2 |
| **Axial Width** | 18 mm | ~14 mm | ~10 mm |
| **Winding Direction** | Clockwise | Counter-CW | Counter-CW |
| **Target Inductance** | 6.33 mH | ≥10 mH | ~1 mH |

### 1.2 Derived Winding Parameters

**Wire pitch (traverse per turn):**
- 0.52 mm wire: 0.52 mm traverse per turn (close wound)
- 0.32 mm wire: 0.32 mm traverse per turn (close wound)

**Calculated turns per layer:**

| Coil | Width | Wire | Turns/Layer |
|------|-------|------|-------------|
| TX | 18 mm | 0.52 mm | 34 |
| RX | 14 mm | 0.32 mm | 43 |
| Bucking | 10 mm | 0.52 mm | 19 |

---

## 2. Design Overview

### 2.1 Design Philosophy

**Guiding Principles:**
1. **Use available components** - Arduino kit + DTU component shop
2. **Servo-driven traverse** - Simple PWM control, no stepper driver needed
3. **Accurate turn counting** - ITR8307 optical sensor with interrupt
4. **Auto-reverse at width limits** - No manual intervention during layer winding
5. **Embedded C firmware** - Direct register access, minimal dependencies
6. **Budget conscious** - Under €25 total cost

### 2.2 System Block Diagram

```
┌──────────────────────────────────────────────────────────────────────────┐
│                    COIL WINDER SYSTEM (Servo Traverse)                   │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│   ┌────────────┐    ┌────────────────┐    ┌─────────────────────────┐   │
│   │   WIRE     │    │   TRAVERSE     │    │      SPINDLE            │   │
│   │   SUPPLY   │───►│   CARRIAGE     │───►│      ASSEMBLY           │   │
│   │            │    │                │    │                         │   │
│   └────────────┘    └────────────────┘    └─────────────────────────┘   │
│         │                   │                        │                   │
│         ▼                   ▼                        ▼                   │
│   ┌────────────┐    ┌────────────────┐    ┌─────────────────────────┐   │
│   │  Spool     │    │  Hobby Servo   │    │  Hand Crank             │   │
│   │  Holder    │    │  + Push Rod    │    │  + Optical Encoder      │   │
│   │  Tensioner │    │  + Slide Rail  │    │  + Adapter Plates       │   │
│   └────────────┘    └────────────────┘    └─────────────────────────┘   │
│                             │                        │                   │
│                             ▼                        ▼                   │
│                     ┌────────────────────────────────────────────┐      │
│                     │           ARDUINO UNO CONTROLLER           │      │
│                     │  • Turn counting (INT0 interrupt)          │      │
│                     │  • Servo PWM (Timer1 OC1A)                 │      │
│                     │  • Auto-reverse at width limits            │      │
│                     │  • OLED display (TWI direct)               │      │
│                     │  • Embedded C firmware                     │      │
│                     └────────────────────────────────────────────┘      │
│                                                                          │
└──────────────────────────────────────────────────────────────────────────┘
```

### 2.3 Top-Level Assembly Drawing

```
                               FRONT VIEW (Servo Traverse)
    ┌──────────────────────────────────────────────────────────────────────┐
    │                                                                      │
    │   Wire Spool          Servo + Linkage        Coil Form   Hand Crank │
    │      ○                     ┌───┐             ┌─────┐        ╱       │
    │     ╱│╲                    │SRV│             │     │       ○        │
    │    ╱ │ ╲                   └─┬─┘             │ TX  │      ╱│        │
    │   ●──┼──● ──────────────────┼───────────►   │Coil │ ◄───╱ │        │
    │    ╲ │ ╱                    │    Wire       │     │    Spindle     │
    │     ╲│╱    Tensioner     ┌──┴──┐  Guide     └─────┘       │        │
    │      ○                   │Guide│                          │        │
    │                          │Slide│                          │        │
    │                          └─────┘                          │        │
    │                            ↕                              │        │
    │                     ◄── 18mm ──►                          │        │
    │                     (servo sweep)                         │        │
    │                                                                     │
    │   ╔═══════════════════════════════════════════════════════════════╗│
    │   ║                    BASE PLATE (3D Printed)                    ║│
    │   ╚═══════════════════════════════════════════════════════════════╝│
    │                                                                     │
    │   [  OLED DISPLAY  ]                             [RST] [SET] [GO]  │
    │   ┌────────────────┐                                               │
    │   │ TX  18mm  [RUN]│                                               │
    │   │ 034/068  L1/2  │                                               │
    │   │ >>> 9.4/18mm   │                                               │
    │   │ [████████] 50% │                                               │
    │   └────────────────┘                                               │
    │                                                                     │
    └──────────────────────────────────────────────────────────────────────┘
```

### 2.4 How Servo Traverse Works

```
                        TRAVERSE MOTION DIAGRAM

    Layer 1: Left → Right                    Layer 2: Right → Left
    ┌────────────────────────┐               ┌────────────────────────┐
    │ START                  │               │                   START│
    │  ↓                     │               │                     ↓  │
    │  ○○○○○○○○○○○○○○○○○○→  │               │  ←○○○○○○○○○○○○○○○○○○  │
    │                    END │               │ END                    │
    └────────────────────────┘               └────────────────────────┘
           ════════════════                         ════════════════
              Coil Form                                Coil Form

    For each spindle rotation:
    1. ITR8307 detects slot passing (INT0 interrupt)
    2. Firmware increments turn count
    3. Servo angle updated by wire diameter amount
    4. At width limit: reverse direction, increment layer count
```

**Servo Linkage:**
```
    SERVO
      │
      +── Horn (15mm)
           │
           +── Push rod (rigid wire)
                │
                +── Wire guide carriage (slides on rail)

    Servo rotation: ~50° = ~18mm linear travel
```

---

## 3. Mechanical Design

### 3.1 Frame Design

**Base Plate:**
- Dimensions: 300 mm × 150 mm × 8 mm
- Material: 3D printed PLA
- Features: Mounting holes for all components

**Vertical Supports (2x):**
- Height: 120 mm
- Features: Bearing housings for spindle

```
                    SIDE VIEW - FRAME

         Bearing Housing              Bearing Housing
              ┌───┐                       ┌───┐
              │ ○ │───────────────────────│ ○ │──► Spindle
              └─┬─┘                       └─┬─┘
                │                           │
        ┌───────┴───────┐           ┌───────┴───────┐
        │    LEFT       │           │    RIGHT      │
        │   SUPPORT     │           │   SUPPORT     │
        │   (3D Print)  │           │   (3D Print)  │
        └───────┬───────┘           └───────┬───────┘
                │                           │
    ════════════╧═══════════════════════════╧════════════
                        BASE PLATE
```

### 3.2 Spindle Assembly

**Main Spindle:**
- M8 threaded rod, 300 mm length
- Supported by 608ZZ bearings (8mm ID, 22mm OD)
- One end: Hand crank attachment
- Other end: Optical encoder wheel

**Adapter Plate System:**
Interchangeable 3D printed plates for each coil size:

```
    200mm ADAPTER           120mm ADAPTER           80mm ADAPTER
    ┌─────────────┐         ┌─────────────┐        ┌─────────────┐
    │ ┌─────────┐ │         │  ┌───────┐  │        │   ┌─────┐   │
    │ │  200mm  │ │         │  │120mm  │  │        │   │80mm │   │
    │ │  FORM   │ │         │  │ FORM  │  │        │   │FORM │   │
    │ │ RECESS  │ │         │  └───────┘  │        │   └─────┘   │
    │ └─────────┘ │         │      ○      │        │      ○      │
    │      ○      │         │   (M8 Hub)  │        │   (M8 Hub)  │
    └──────┴──────┘         └──────┴──────┘        └──────┴──────┘
```

### 3.3 Servo Traverse Mechanism

The servo provides simple, reliable traverse motion without complex stepper drivers.

**Components:**

| Component | Source | Purpose |
|-----------|--------|---------|
| Hobby Servo | Your Kit | Traverse drive |
| 3D Printed Linkage | DTU Lab | Motion transfer |
| Simple Slide Rail | 3D Printed | Linear guide |
| Wire Guide Eyelet | Hardware Store | Wire path |

**Servo Linkage Design:**

```
                    SIDE VIEW - SERVO TRAVERSE

                       Servo Horn (15mm)
                            │
    [SERVO]────────────────┼────────────┐
                           │            │
                           ▼            │ Push Rod
                      ┌────────┐        │
                      │        │◄───────┘
                      │ Guide  │
                      │Carriage│ ═══════ Slides on 3D printed rail
                      └────┬───┘
                           │
                      Wire Guide
                           │
                           ▼
                      [Coil Form]
```

**Servo Calculation:**
```
Servo range: 50° rotation (±25° from center)
Horn length: 15mm
Arc travel: 15mm × sin(25°) × 2 ≈ 12.7mm per side

For 18mm traverse: Use 25° range or adjust horn length
Actual calibration done in firmware (SERVO_RANGE define)
```

### 3.4 Encoder Wheel Design (for Reflective Sensor)

Since the ITR8307 is a **reflective** sensor, the encoder wheel needs a reflective stripe:

```
    ┌─────────────────┐
    │  ████████████   │    Dark/black surface
    │  ████████████   │    (non-reflective, matte)
    │                 │
    │   ┌───────┐     │
    │   │ WHITE │     │    White/reflective stripe
    │   │STRIPE │     │    (triggers sensor once per revolution)
    │   └───────┘     │
    │                 │
    │        ●        │    Center hole (fits M8 spindle)
    │                 │
    └─────────────────┘

    Diameter: ~40mm
    Stripe width: ~5mm
    Stripe arc length: ~10-15mm
    Detection distance: 1-3mm from sensor face
```

**Implementation options:**
1. Black wheel with white tape stripe
2. Black wheel with white paint stripe
3. Dark PLA wheel with white PLA insert
4. Print in black, sand one stripe area and apply white paint

**Signal behavior:**
- Dark surface: Output HIGH (~5V)
- White stripe passes: Output LOW (~0V)
- INT0 triggers on falling edge

### 3.5 Wire Tensioner

```
                    TENSIONER DETAIL

    Wire from Spool
          │
          ▼
    ┌───────────┐
    │  FELT PAD │◄── Adjustable pressure
    ├───────────┤    (spring loaded)
    │   WIRE    │───► To servo carriage
    ├───────────┤
    │  FELT PAD │
    └───────────┘
```

---

## 4. Electronics Design

### 4.1 System Architecture

```
┌───────────────────────────────────────────────────────────────────────────┐
│                    ELECTRONICS BLOCK DIAGRAM (Servo Version)              │
├───────────────────────────────────────────────────────────────────────────┤
│                                                                           │
│  ┌──────────────┐                    ┌─────────────────────────────────┐ │
│  │   ITR8307    │         PD2 (INT0) │         ARDUINO UNO             │ │
│  │   OPTICAL    │───────────────────►│         (ATmega328P)            │ │
│  │   SENSOR     │                    │                                 │ │
│  └──────────────┘                    │  PD2:  Encoder input (INT0)     │ │
│                                      │  PB1:  Servo PWM (OC1A/D9)      │ │
│  ┌──────────────┐                    │  PD3:  Reset button             │ │
│  │    OLED      │◄───────────────────│  PD4:  Preset button            │ │
│  │   DISPLAY    │  I2C (PC4/PC5)     │  PD5:  Start button             │ │
│  │  (SSD1306)   │                    │  PB2:  Buzzer (D10)             │ │
│  └──────────────┘                    │  PC4:  I2C SDA (A4)             │ │
│                                      │  PC5:  I2C SCL (A5)             │ │
│  ┌──────────────┐                    │                                 │ │
│  │   BUTTONS    │───────────────────►│                                 │ │
│  │  RST/SET/GO  │  PD3,PD4,PD5       └─────────────────────────────────┘ │
│  └──────────────┘                                 │                      │
│                                                   │ PB1 (D9)             │
│  ┌──────────────┐                                 ▼                      │
│  │    SERVO     │◄────────────────────────────────                       │
│  │   MOTOR      │  PWM (1-2ms pulse, 50Hz)                              │
│  └──────────────┘                                                        │
│                                                                           │
│  POWER:                                                                   │
│  • Arduino: USB (5V)                                                      │
│  • Servo: 5V from Arduino (add 100µF capacitor)                          │
│  • No external power supply needed!                                       │
│                                                                           │
└───────────────────────────────────────────────────────────────────────────┘
```

### 4.2 Turn Counting - ITR8307 Optical Sensor

**Sensor: ITR8307/F43 (from DTU Component Shop)**
- **REFLECTIVE optical sensor** (not slot/interrupter!)
- Both LED and detector point in same direction
- Detects light bouncing back from reflective surface
- Optimal detection distance: 1-3mm
- Operating voltage: 5V
- Output: Open collector NPN phototransistor

**Correct Pinout (from datasheet):**
```
    Pin 1 = CATHODE  (LED -)
    Pin 2 = ANODE    (LED +)
    Pin 3 = COLLECTOR (Detector output)
    Pin 4 = EMITTER  (Detector -)
```

**Circuit:**

```
                         5V
                          │
            ┌─────────────┼─────────────┐
            │             │             │
          [330Ω]        [10kΩ]       [100nF] ← Essential for noise!
            │             │             │
            │             ├─────────────┼──► PD2 (D2/INT0)
            │             │             │
    Pin 2 ──┘    Pin 3 ───┘             │
    (Anode)      (Collector)            │
                                        │
    Pin 1 ──┬── Pin 4 ──────────────────┴──► GND
    (Cathode)   (Emitter)

    Components from DTU: ITR8307, 330Ω (330R), 10kΩ (10K0), 100nF
```

> [!warning] Encoder Wheel Design
> Since this is a **reflective** sensor, use an encoder wheel with a **white reflective stripe** on a dark background, not a slotted wheel!

### 4.3 Servo Control

**PWM via Timer1:**
- 50Hz (20ms period)
- 1ms pulse = 0° position
- 1.5ms pulse = 90° center
- 2ms pulse = 180° position

```
    Timer1 Configuration:
    - Fast PWM mode, TOP = ICR1
    - ICR1 = 39999 (20ms at 16MHz/8 prescaler)
    - OCR1A controls pulse width (2000-4000 counts)
```

### 4.4 Display - SSD1306 OLED

**Direct TWI Register Access:**
- No library dependencies
- Custom 5x7 font built into firmware
- ~1KB display buffer

**Display Layout:**
```
┌────────────────────────────────────────┐
│  TX 18mm             [RUN]             │
│  068/068  L2/2                         │
│  >>> 18.0/18mm                         │
│  [████████████████████] 100%           │
└────────────────────────────────────────┘
```

### 4.5 Pin Assignment Summary

| ATmega328P Pin | Arduino | Function | Direction |
|----------------|---------|----------|-----------|
| PD2 | D2 | Encoder input | Input (INT0) |
| PD3 | D3 | Reset button | Input (pull-up) |
| PD4 | D4 | Preset button | Input (pull-up) |
| PD5 | D5 | Start button | Input (pull-up) |
| PB1 | D9 | Servo PWM | Output (OC1A) |
| PB2 | D10 | Buzzer | Output |
| PC4 | A4 | I2C SDA | I/O |
| PC5 | A5 | I2C SCL | Output |

### 4.6 Wiring Diagram

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
    BUZZER ─────────►│ D10       │
                     │           │
    OLED SDA ───────►│ A4        │
    OLED SCL ───────►│ A5        │
                     │           │
                     │ 5V ───────┼──► Power
                     │ GND ──────┼──► Ground
                     └───────────┘
```

---

## 5. Component Sources

### 5.1 From DTU Component Shop (Free)

| Component | DTU Part | Qty |
|-----------|----------|-----|
| ITR8307 | ITR8307 | 1 |
| 330Ω Resistor | 330R (E96) | 1 |
| 10kΩ Resistor | 10K0 (E96) | 1 |
| 100nF Capacitor | 100n Ceramic | 2 |
| 100µF Capacitor | 100µF Electrolytic | 1 |
| Pushbuttons | Tryk knap til bredbord | 3 |
| Pin Headers | Pin header strips | 1 |

### 5.2 From Your Arduino Kit

| Component | Reason |
|-----------|--------|
| Arduino UNO | No microcontrollers at DTU |
| Servo Motor | No servos at DTU |
| Piezo Buzzer | No buzzers at DTU |
| Jumper Wires | Convenient |

### 5.3 From Your Parts

| Component |
|-----------|
| OLED Display 128x64 |

### 5.4 Hardware Store (~€10)

| Component | Qty | Cost |
|-----------|-----|------|
| M8 Threaded Rod 30cm | 1 | €2 |
| 608ZZ Bearings | 2 | €3 |
| M8 Nuts | 6 | €1 |
| M3 Bolts/Nuts | 20 | €2 |

### 5.5 3D Printing (~€15)

| Part | Filament |
|------|----------|
| Base plate | ~150g |
| Bearing mounts (2) | ~60g |
| Coil adapters (3) | ~80g |
| Hand crank | ~20g |
| Encoder wheel | ~5g |
| Servo mount + linkage | ~25g |
| Wire guide carriage | ~15g |
| **Total** | **~355g** |

**Total Cost: ~€25**

---

## 6. 3D Printed Parts

### 6.1 Parts List

| Part | Quantity | Print Time | Notes |
|------|----------|------------|-------|
| Base Plate | 1 | ~6 hours | Main frame |
| Bearing Mounts | 2 | ~2 hours | 608ZZ housing |
| 200mm Adapter | 1 | ~3 hours | TX coil |
| 120mm Adapter | 1 | ~2 hours | Bucking coil |
| 80mm Adapter | 1 | ~1 hour | RX coil |
| Hand Crank | 1 | ~1 hour | |
| Encoder Wheel | 1 | ~30 min | 100% infill |
| Servo Mount | 1 | ~1 hour | |
| Linkage Arm | 1 | ~30 min | |
| Wire Guide Carriage | 1 | ~1 hour | |
| Slide Rail | 1 | ~1 hour | |

**Total Print Time:** ~18 hours

### 6.2 Print Settings

| Setting | Value |
|---------|-------|
| Layer Height | 0.2 mm |
| Infill | 20% (100% for encoder wheel) |
| Material | PLA |
| Supports | Minimal |

---

## 7. Firmware

### 7.1 Implementation

**Language:** Embedded C (no Arduino framework)
**Location:** `coil_winder/src/main.c`

**Key Features:**
- Direct AVR register access
- Timer0 for millisecond timing
- Timer1 for servo PWM (OC1A)
- INT0 for turn counting
- TWI for OLED communication
- Custom 5x7 font renderer
- ~4KB Flash, ~1KB RAM

### 7.2 Build with PlatformIO

```bash
cd coil_winder

# Build
pio run

# Upload
pio run -t upload
```

### 7.3 Calibration

Edit these defines in `src/main.c`:

```c
#define SERVO_CENTER    90   /* Center position (degrees) */
#define SERVO_RANGE     25   /* Range each side (degrees) */
```

---

## 8. Assembly Instructions

### 8.1 Build Sequence

1. **3D print all parts** (~18 hours)
2. **Get components from DTU** (ITR8307, resistors, caps, buttons)
3. **Buy hardware store items** (M8 rod, bearings, nuts)
4. **Assemble frame** (base, supports, spindle)
5. **Install servo traverse** (mount, linkage, slide)
6. **Wire electronics** (sensor, servo, buttons, display)
7. **Upload firmware** (`pio run -t upload`)
8. **Calibrate and test**

### 8.2 Wiring Steps

1. Connect ITR8307 to D2 with 330Ω and 10kΩ
2. Connect servo signal to D9, power to 5V
3. Connect buttons to D3, D4, D5 (other side to GND)
4. Connect OLED: SDA→A4, SCL→A5, VCC→5V, GND→GND
5. Connect buzzer to D10 (optional)
6. Add 100µF capacitor between 5V and GND near servo

---

## 9. Operation

### 9.1 Controls

| Button | Action |
|--------|--------|
| RESET (D3) | Zero counter, home servo |
| PRESET (D4) | Cycle TX → RX → BUCK |
| START (D5) | Start/Stop winding |

### 9.2 Winding Procedure

1. Press PRESET to select coil type
2. Mount correct adapter plate
3. Thread wire through guide
4. Press START
5. Turn hand crank steadily
6. Servo automatically traverses
7. Buzzer sounds when complete

### 9.3 Display Shows

- Coil type and width
- Current turn / target turns
- Current layer / total layers
- Traverse position and direction
- Progress bar with percentage

---

## 10. Troubleshooting

| Problem | Cause | Solution |
|---------|-------|----------|
| Sensor gets hot | Wrong LED polarity | Pin 1 (Cathode) must go to GND |
| No turn count | Sensor wiring or no reflective surface | Use white paper at 1-3mm distance |
| Always HIGH | No reflection detected | Check LED working (phone camera sees IR) |
| Always LOW | Too much ambient light | Shield sensor, check wiring |
| Double counting | Noise/vibration | Add 100nF cap on D2, increase debounce |
| Noisy signal | Servo causing interference | Add 100µF cap on servo power |
| Display blank | I2C issue | Check SDA/SCL, try 0x3D address |
| Servo jitters | Power supply noise | Add 100µF cap on 5V near servo |
| Wrong traverse | Calibration | Adjust SERVO_RANGE in code |
| Buttons stop working | State tracking bug | Update to firmware v2.0 |

> [!tip] Detailed Debugging
> See [[Coil_Winder_Circuit_Notes]] for oscilloscope traces, detailed debugging steps, and code review findings.

---

## 11. Related Documents

- [[Coil Design]] - Coil specifications
- [[Coil_Winder_BOM]] - Full bill of materials
- [[Coil_Winder_Circuit_Notes]] - Circuit debugging, noise issues, code review

---

*Coil Winder Design for DTU 34621 Metal Detector Project*
*Simplified servo-based design with embedded C firmware*
*Total cost: ~€25*
*Last updated: 2026-01-12*
