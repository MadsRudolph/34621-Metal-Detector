<div align="center">

# VLF Metal Detector

**Electromagnetic Sensors & Digital Signal Processing**

[![Status](https://img.shields.io/badge/Status-In%20Progress-green?style=for-the-badge)](https://github.com)
[![Course](https://img.shields.io/badge/DTU-34621-red?style=for-the-badge)](https://www.dtu.dk)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20Mega-00979D?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-Educational-blue?style=for-the-badge)](LICENSE)

<br>

*A Very Low Frequency induction balance metal detector with real-time DFT-based phase detection*

[Features](#-features) · [Hardware](#-hardware) · [Getting Started](#-getting-started) · [Progress](#-progress) · [Team](#-team)

---

</div>

> [!NOTE]
> This project is in **active development**. Coil design is finalized (Ø200mm), circuit simulations are validated (~90% efficiency), and firmware is being prepared for hardware testing.

<br>

## ✨ Features

<table>
<tr>
<td width="50%">

### Signal Processing
- Single-bin DFT optimized for 4× oversampling
- No trigonometric calculations needed
- IIR filtering for stable readings
- 8 kHz sample rate, 64-sample window

</td>
<td width="50%">

### Metal Detection
- Phase-based ferrous/non-ferrous classification
- 65° threshold discrimination
- Real-time OLED display
- Audio feedback via buzzer

</td>
</tr>
</table>

<br>

## 🔧 System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              TX PATH                                        │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │   MCU   │───▶│ Op-Amp  │───▶│ AA Filt │───▶│ TX Coil │                  │
│  │ 2kHz PWM│    │ (drive) │    │  (LPF)  │    │    ○    │                  │
│  └─────────┘    └─────────┘    └─────────┘    └────┬────┘                  │
│       ▲                                            │                        │
│       │                                        [Metal]                      │
│       │                                            │                        │
│       │                         RX PATH            ▼                        │
│  ┌────┴────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │ MCP3208 │◀───│ LP Filt │◀───│ Op-Amp  │◀───│ RX Coil │                  │
│  │ 12b ADC │    │         │    │  (amp)  │    │    ○    │                  │
│  └────┬────┘    └─────────┘    └─────────┘    └─────────┘                  │
│       │                                                                     │
│       ▼              DIGITAL SIGNAL PROCESSING                              │
│  ┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐                  │
│  │   DFT   │───▶│   IIR   │───▶│Classify │───▶│ Display │                  │
│  │  Mag/φ  │    │ Filter  │    │  Fe/NFe │    │ + Audio │                  │
│  └─────────┘    └─────────┘    └─────────┘    └─────────┘                  │
└─────────────────────────────────────────────────────────────────────────────┘
```

<br>

## 🛠 Hardware

<table>
<tr>
<th>Component</th>
<th>Part</th>
<th>Description</th>
</tr>
<tr>
<td><b>Microcontroller</b></td>
<td>ATmega2560</td>
<td>Arduino Mega 2560</td>
</tr>
<tr>
<td><b>ADC</b></td>
<td>MCP3208</td>
<td>12-bit, 8-channel, SPI</td>
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
| Coil Diameter | `200 mm` | Concentric design |
| TX Frequency | `2 kHz` | PWM generated |
| Sample Rate | `8 kHz` | 4× oversampling |
| DFT Window | `64 samples` | ~8 ms |
| Phase Threshold | `65°` | Fe vs Non-Fe |
| Min Inductance | `10 mH` | RX coil requirement |
| H-bridge Efficiency | `~90%` | QSPICE validated |
| Runtime | `100 min` | @ 9V battery |

<br>

## 📁 Repository Structure

```
📦 34621-Metal-Detector
├── 📂 Code/                  # Main firmware (PlatformIO)
│   ├── 📂 src/
│   │   ├── 📄 main.c
│   │   ├── 📂 signal/        # TX, RX, DFT, filtering
│   │   ├── 📂 app/           # Detection, UI, display
│   │   └── 📂 drivers/       # I2C, SSD1306
│   └── 📄 platformio.ini
├── 📂 Code_PowerTest/        # Power consumption testing firmware
├── 📂 KiCad/                 # PCB schematic & layout
├── 📂 LTspice/               # LTspice circuit simulations
├── 📂 QSPICE/                # QSPICE simulations (H-bridge driver)
├── 📂 Docs/                  # Generated documentation
├── 📂 Literature/            # Datasheets & references
├── 📂 Notes/                 # Design notes & calculations
├── 📂 Meeting/               # Meeting minutes
└── 📄 README.md
```

<br>

## 🚀 Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- Arduino Mega 2560
- USB cable

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
| 9 | PWM → TX Coil | OUT |
| 10 | MCP3208 CS | OUT |
| 50 | SPI MISO | IN |
| 51 | SPI MOSI | OUT |
| 52 | SPI SCK | OUT |
| 20 | I2C SDA | I/O |
| 21 | I2C SCL | OUT |
| 2 | Start/Stop Button | IN |
| 3 | Calibrate Button | IN |
| 8 | Buzzer | OUT |

</details>

<br>

## 📊 Progress

| Phase | Status | Progress |
|:------|:------:|:---------|
| Requirements & Planning | ✅ Done | ████████████ 100% |
| Coil Design | ✅ Done | ████████████ 100% |
| Circuit Design | 🔄 Active | ██████████░░ 85% |
| QSPICE Simulations | ✅ Done | ████████████ 100% |
| Firmware Development | 🔄 Active | ██████████░░ 85% |
| PCB Schematic | 🔄 Active | ██████░░░░░░ 50% |
| Hardware Build | ⏳ Pending | ░░░░░░░░░░░░ 0% |
| Testing & Calibration | ⏳ Pending | ░░░░░░░░░░░░ 0% |
| Final Report | 🔄 Active | ████░░░░░░░░ 30% |

> **Deadline:** January 23, 2026
>
> **Next milestone:** Prototype software ready for testing (Week 2), Full hardware testing (Week 3)

<br>

## 👥 Team

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
