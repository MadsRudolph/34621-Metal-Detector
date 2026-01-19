<div align="center">

# VLF Metal Detector

**Electromagnetic Sensors & Digital Signal Processing**

[![Status](https://img.shields.io/badge/Status-In%20Progress-green?style=for-the-badge)](https://github.com)
[![Course](https://img.shields.io/badge/DTU-34621-red?style=for-the-badge)](https://www.dtu.dk)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20Nano-00979D?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-Educational-blue?style=for-the-badge)](LICENSE)

<br>

*A Very Low Frequency induction balance metal detector with real-time DFT-based phase detection*

[Features](#features) | [Hardware](#hardware) | [Getting Started](#getting-started) | [Progress](#progress) | [Team](#team)

---

</div>

> [!NOTE]
> This project is in **active development**. Core firmware is feature-complete with modular architecture, IIR filtering, buzzer feedback, and metal classification. H-bridge driver and RX amplifier are designed and simulated. Hardware build and integration testing in progress.

<br>

## Features

<table>
<tr>
<td width="50%">

### Signal Processing
- Single-bin DFT optimized for 4x oversampling
- No trigonometric calculations needed (coefficients: +1, -1, 0)
- 8 kHz sample rate, 64-sample window
- Real-time magnitude and phase calculation
- IIR filtering for stable readings
- Phase-based metal classification (ferro/non-ferro)

</td>
<td width="50%">

### Hardware Design
- H-bridge MOSFET driver (IRF5305PbF + IRL530)
- RX amplifier circuit (BC337 transistor)
- Concentric coil configuration with bucking coil
- Timer-synchronized ADC sampling
- OLED display output (SSD1306)
- Buzzer for audio feedback

</td>
</tr>
</table>

<br>

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              TX PATH                                        │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐                                 │
│  │   MCU   │───>│ H-Bridge│───>│ TX Coil │                                 │
│  │ 2kHz SQ │    │ MOSFET  │    │  200mm  │                                 │
│  └─────────┘    └─────────┘    └────┬────┘                                 │
│       ^                             │                                       │
│       │                         [Metal]                                     │
│       │                             │                                       │
│       │              RX PATH        v                                       │
│  ┌────┴────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │ 10-bit  │<───│ Bucking │<───│ RX Coil │<───│ RX Amp  │                  │
│  │   ADC   │    │  Coil   │    │  80mm   │    │ BC337   │                  │
│  └────┬────┘    └─────────┘    └─────────┘    └─────────┘                  │
│       │                                                                     │
│       v              DIGITAL SIGNAL PROCESSING                              │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │   DFT   │───>│IIR Filt │───>│ Detect  │───>│ Display │                  │
│  │  Re/Im  │    │ Smooth  │    │ Classify│    │ + Buzzer│                  │
│  └─────────┘    └─────────┘    └─────────┘    └─────────┘                  │
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
34621-Metal-Detector/
├── Code/                     # Main firmware (PlatformIO)
│   └── src/
│       ├── main.c            # Main program loop
│       ├── adc.c             # ADC auto-trigger sampling
│       ├── button.c          # Button handling with debounce
│       ├── buzzer.c          # Audio feedback (beeps)
│       ├── capture.c         # MATLAB serial interface
│       ├── detection.c       # Metal classification
│       ├── dft.c             # DFT calculation
│       ├── display.c         # OLED output with graphical HUD
│       ├── filter.c          # IIR filtering
│       ├── jingle.c          # Startup melody
│       ├── timer.c           # Timer0/Timer1 setup
│       ├── include/          # Header files
│       │   ├── config.h      # Global configuration
│       │   └── *.h           # Module headers
│       └── drivers/          # I2C, SSD1306 drivers
├── Docs/                     # Project documentation
│   ├── Theory/               # Technical analysis
│   ├── Guides/               # Implementation guides
│   ├── Matlab/               # MATLAB verification scripts
│   └── Project_Roadmap.md    # Requirements tracking
├── KiCad/                    # PCB schematics
│   ├── Metaldetector/        # H-bridge and MCU
│   └── rx forstærker/        # RX amplifier circuit
├── LTspice/                  # LTspice simulations
├── QSPICE/                   # QSPICE simulations (H-bridge)
└── Literature/               # Datasheets & references
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
| 11 (PB3) | Buzzer output (PWM tone) | OUT |
| A0 (PC0) | RX signal input (ADC) | IN |
| A4 (PC4) | I2C SDA (OLED) | I/O |
| A5 (PC5) | I2C SCL (OLED) | OUT |
| D2 (PD2) | Start/Stop button | IN |
| D3 (PD3) | Calibration button | IN |
| D4 (PD4) | Debug display toggle | IN |

</details>

<br>

## Progress

| Phase | Status | Progress |
|:------|:------:|:---------|
| Requirements & Planning | Done | 100% |
| Coil Design | Done | 100% |
| Circuit Design (H-bridge) | Done | 100% |
| RX Amplifier Design | Done | 100% |
| QSPICE/LTspice Simulations | Done | 100% |
| Core Firmware (TX/RX/DFT) | Done | 100% |
| IIR Filtering | Done | 100% |
| Metal Classification | Done | 100% |
| UI Firmware (Buttons/Buzzer) | Done | 100% |
| Code Modularization | Done | 100% |
| Hardware Build | Active | 40% |
| Integration Testing | Pending | 0% |
| Final Report | Active | 50% |

**Overall Progress: ~75%** (Software 95%, Hardware 40%)

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
