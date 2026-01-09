# DTU 34621 - Metal Detector Project

> **Course:** 34621 Electromagnetic Sensors and Digital Signal Processing
> **Institution:** DTU Diplom
> **Project:** VLF Metal Detector
> **Team:** Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech

---

## Documentation

### Guides
| Document | Description |
|----------|-------------|
| [[Docs/Guides/Code_Review\|Code Review]] | Firmware arkitektur og kode gennemgang |
| [[Docs/Guides/Assembly_Guide\|Assembly Guide]] | Hardware opsætning og wiring |
| [[Docs/Guides/Test_Guide_AD3\|Test Guide (AD3)]] | Test med Analog Discovery 3 |

### Theory
| Document | Description |
|----------|-------------|
| [[Docs/Theory/DFT Algorithm\|DFT Algorithm]] | 4x oversampling DFT forklaring |
| [[Docs/Theory/Coil Design\|Coil Design]] | Spole specifikationer (MATLAB-verificeret) |
| [[Docs/Theory/Coil Configuration Comparison\|Coil Configurations]] | Concentric vs Double-D sammenligning |
| [[Docs/Theory/TX Driver and Tank Circuit Design\|TX Driver Design]] | Driver kredsløb og tank circuit |
| [[Docs/Theory/Power Budget Analysis\|Power Budget]] | Strømforbrug analyse |
| [[Docs/Theory/Theory References\|Theory References]] | Links til DTU kursusmateriale |

### Other
| Section | Description |
|---------|-------------|
| [[Code/README\|Code]] | Arduino Nano firmware |
| [[Docs/Meetings\|Meetings]] | Mødereferater |
| [[Literature/README\|Literature]] | Datasheets og kursusmateriale |

---

## Project Requirements (Kravspecifikation)

### Priority 1 (Must Have)
- [ ] **1.** Amplitude/phase detection
- [ ] **2.** Distinguish ferrous/non-ferrous metals
- [ ] **3.** Detect iron (r=15mm, l=50mm) at 50mm depth
- [ ] **4.** 9V battery powered (single 6LR61)
- [ ] **5.** 100 min runtime, >6V remaining
- [ ] **6a.** Arduino-based processor
- [ ] **6b.** Discrete components for coil drive/amp
- [ ] **8a.** Display amplitude & phase
- [ ] **8b.** Display readability (stable values)
- [ ] **9a.** Start/stop button
- [ ] **9b.** Calibration button (zero when no metal)
- [ ] **10.** VLF detection principle
- [ ] **11.** Sampling frequency 8 kHz
- [ ] **12.** Detection frequency 2 kHz
- [ ] **16.** RX coil inductance >= 10 mH

### Priority 2 (Should Have)
- [ ] **7.** Timer interrupt ADC control
- [ ] **8c.** IIR filter for display smoothing
- [ ] **13.** Enclosure
- [ ] **14.** Coil mount
- [ ] **15.** Centered coil design (TX outer, RX inner)

---

## System Architecture

```
                          TX PATH
┌─────────┐    ┌─────────┐    ┌─────────┐
│  Timer0 │───>│  Pin 9  │───>│ TX Coil │
│  8kHz   │    │  2kHz   │    │  15mH   │
└─────────┘    └─────────┘    └────┬────┘
     │                             │
     │                        [Metal]
     │                             │
     │              RX PATH        v
┌────┴────┐    ┌─────────┐    ┌─────────┐
│   ADC   │<───│  Pin A0 │<───│ RX Coil │
│  10-bit │    │  8kHz   │    │  10mH   │
└────┬────┘    └─────────┘    └─────────┘
     │
     v
┌─────────┐    ┌─────────┐    ┌─────────┐
│   DFT   │───>│  Mag/   │───>│  OLED   │
│ 64 samp │    │  Phase  │    │ Display │
└─────────┘    └─────────┘    └─────────┘
```

---

## Key Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| MCU | ATmega328P | Arduino Nano |
| TX Frequency | 2 kHz | Timer0 generated |
| Sample Rate | 8 kHz | 4x oversampling |
| ADC Resolution | 10-bit | Internal ADC |
| DFT Window | 64 samples | 8ms window |
| Phase Threshold | 65 deg | Ferrous/non-ferrous |

---

## External Resources

- [[Literature/Automotive-Microcontrollers-ATmega328P_Datasheet.pdf|ATmega328P Datasheet]]
- [SSD1306 OLED Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

---

## Timeline

- **Final report deadline:** 23/1-2026

