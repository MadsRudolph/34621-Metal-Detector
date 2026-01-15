<div align="center">

# VLF Metal Detector

**Electromagnetic Sensors & Digital Signal Processing**

[![Status](https://img.shields.io/badge/Status-In%20Progress-green?style=for-the-badge)](https://github.com)
[![Course](https://img.shields.io/badge/DTU-34621-red?style=for-the-badge)](https://www.dtu.dk)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20Nano-00979D?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-Educational-blue?style=for-the-badge)](LICENSE)

<br>

*A Very Low Frequency induction balance metal detector with real-time DFT-based phase detection*

[Features](#-features) · [Hardware](#-hardware) · [Getting Started](#-getting-started) · [Progress](#-progress) · [Team](#-team)

---

</div>

> [!NOTE]
> This project is in **active development**. Core signal processing firmware is working, concentric coil design is finalized (TX Ø200mm, RX Ø80mm), H-bridge driver is designed and simulated (~90% efficiency). Hardware build and integration testing pending.

<br>

## Features

<table>
<tr>
<td width="50%">

### Signal Processing
- Single-bin DFT optimized for 4× oversampling
- No trigonometric calculations needed (coefficients: +1, -1, 0)
- 8 kHz sample rate, 64-sample window
- Real-time magnitude and phase calculation

</td>
<td width="50%">

### Hardware Design
- H-bridge MOSFET driver (IRF5305PbF + IRL530)
- Concentric coil configuration with bucking coil
- Timer-synchronized ADC sampling
- OLED display output (SSD1306)

</td>
</tr>
</table>

<br>

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              TX PATH                                        │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                                 │
│  │   MCU   │───▶│ H-Bridge│───▶│ TX Coil │                                 │
│  │ 2kHz SQ │    │ MOSFET  │    │ Ø200mm  │                                 │
│  └─────────┘    └─────────┘    └────┬────┘                                 │
│       ▲                             │                                       │
│       │                         [Metal]                                     │
│       │                             │                                       │
│       │              RX PATH        ▼                                       │
│  ┌────┴────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │ 10-bit  │◀───│ Bucking │◀───│ RX Coil │◀───│ LPF/Amp │                  │
│  │   ADC   │    │  Coil   │    │  Ø80mm  │    │         │                  │
│  └────┬────┘    └─────────┘    └─────────┘    └─────────┘                  │
│       │                                                                     │
│       ▼              DIGITAL SIGNAL PROCESSING                              │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                                 │
│  │   DFT   │───▶│ Mag/Ang │───▶│ Display │                                 │
│  │  Re/Im  │    │  Calc   │    │  OLED   │                                 │
│  └─────────┘    └─────────┘    └─────────┘                                 │
└─────────────────────────────────────────────────────────────────────────────┘
```

<br>

## Hardware

<table>
<tr>
<th>Component</th>
<th>Part</th>
<th>Description</th>
</tr>
<tr>
<td><b>Microcontroller</b></td>
<td>ATmega328P</td>
<td>Arduino Nano @ 16MHz</td>
</tr>
<tr>
<td><b>ADC</b></td>
<td>Internal</td>
<td>10-bit, Timer0 auto-trigger @ 8kHz</td>
</tr>
<tr>
<td><b>TX Driver</b></td>
<td>IRF5305 + IRL530</td>
<td>H-bridge MOSFET, ~90% efficiency</td>
</tr>
<tr>
<td><b>Display</b></td>
<td>SSD1306</td>
<td>128×64 OLED, I2C</td>
</tr>
<tr>
<td><b>Power</b></td>
<td>6LR61</td>
<td>9V alkaline battery</td>
</tr>
</table>

### Key Parameters

| Parameter | Value | Notes |
|:----------|:-----:|:------|
| TX Coil | `Ø200 mm` | 20 turns, ~190 µH |
| RX Coil | `Ø80 mm` | 200 turns, ~12.6 mH |
| Bucking Coil | `Ø120 mm` | 20 turns, ~430 µH |
| TX Frequency | `2 kHz` | Timer0 generated square wave |
| Sample Rate | `8 kHz` | 4× oversampling |
| DFT Window | `64 samples` | ~8 ms (16 TX cycles) |
| Detection Depth | `50 mm` | Requirement target |
| H-bridge Efficiency | `~90%` | QSPICE validated |
| Runtime | `100 min` | @ 9V battery |

<br>

## Repository Structure

```
📦 34621-Metal-Detector
├── 📂 Code/                  # Main firmware (PlatformIO)
│   ├── 📂 src/
│   │   ├── 📄 main.c         # All firmware in single file
│   │   └── 📂 drivers/       # I2C, SSD1306 (provided)
│   └── 📄 platformio.ini
├── 📂 Docs/                  # Project documentation
│   ├── 📂 Theory/            # Technical analysis documents
│   ├── 📂 Guides/            # Implementation guides
│   └── 📄 Project_Roadmap.md # Requirements & progress tracking
├── 📂 KiCad/                 # PCB schematic & layout
├── 📂 LTspice/               # LTspice circuit simulations
├── 📂 QSPICE/                # QSPICE simulations (H-bridge)
├── 📂 Literature/            # Datasheets & references
├── 📂 Meeting/               # Meeting minutes
└── 📄 README.md
```

<br>

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- Arduino Nano (ATmega328P)
- USB cable (Mini-B)

### Build & Upload

```bash
# Clone the repository
git clone https://github.com/MadsRudolph/34621-Metal-Detector.git
cd 34621-Metal-Detector/Code

# Build
pio run

# Upload to board
pio run -t upload

# Monitor serial output
pio device monitor
```

<details>
<summary><b>Pin Configuration</b></summary>

<br>

| Pin | Function | Direction |
|:---:|:---------|:---------:|
| 9 (PB1) | TX signal output (2kHz square wave) | OUT |
| A0 (PC0) | RX signal input (ADC) | IN |
| A4 (PC4) | I2C SDA (OLED) | I/O |
| A5 (PC5) | I2C SCL (OLED) | OUT |
| D4 (PD4) | Debug button | IN |
| D2 (PD2) | Start/Stop button (TODO) | IN |
| D3 (PD3) | Calibration button (TODO) | IN |

</details>

<br>

## Progress

| Phase | Status | Progress |
|:------|:------:|:---------|
| Requirements & Planning | ✅ Done | ████████████ 100% |
| Coil Design | ✅ Done | ████████████ 100% |
| Circuit Design (H-bridge) | ✅ Done | ████████████ 100% |
| QSPICE Simulations | ✅ Done | ████████████ 100% |
| Core Firmware (TX/RX/DFT) | ✅ Done | ████████████ 100% |
| UI Firmware (Buttons) | 🔄 Active | ██░░░░░░░░░░ 20% |
| Metal Classification | ⏳ Pending | ░░░░░░░░░░░░ 0% |
| Hardware Build | ⏳ Pending | ░░░░░░░░░░░░ 0% |
| Integration Testing | ⏳ Pending | ░░░░░░░░░░░░ 0% |
| Final Report | 🔄 Active | ████░░░░░░░░ 30% |

**Overall Progress: ~45%** (Software 70%, Hardware 30%)

> **Deadline:** January 23, 2026
>
> See [Project_Roadmap.md](Docs/Project_Roadmap.md) for detailed requirements analysis and implementation status.

<br>

## Team

<table>
<tr>
<td align="center"><b>Mads Rudolph</b></td>
<td align="center"><b>Andreas Skaaning</b></td>
<td align="center"><b>Jonas Beck</b></td>
<td align="center"><b>Sigurd Hestbech</b></td>
</tr>
</table>

<div align="center">

**DTU Diplom** · January 2026

---

<sub>Educational project for DTU course 34621 — Electromagnetic Sensors and Digital Signal Processing</sub>

</div>
