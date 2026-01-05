# 🔍 DTU 34621 - Metal Detector Project

> **Course:** 34621 Electromagnetic Sensors and Digital Signal Processing  
> **Institution:** DTU Diplom  
> **Project:** VLF Metal Detector  
> **Team:** Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech

---

## 📋 Quick Links

| Section | Description |
|---------|-------------|
| [[Code/README\|💻 Code]] | Arduino firmware and DSP implementation |
| [[LTspice/README\|⚡ LTspice]] | Circuit simulations |
| [[KiCad/README\|🔧 KiCad]] | PCB schematics and layout |
| [[Literature/README\|📚 Literature]] | Course materials and datasheets |
| [[Notes/README\|📝 Notes]] | Personal notes and documentation |

---

## 🎯 Project Requirements (Kravspecifikation)

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
- [ ] **11.** Sampling frequency 8 kHz ± 100 Hz
- [ ] **12.** Detection frequency 2 kHz ± 100 Hz
- [ ] **16.** RX coil inductance ≥ 10 mH

### Priority 2 (Should Have)
- [ ] **7.** Timer interrupt ADC control
- [ ] **8c.** FIR/IIR filter for display smoothing
- [ ] **13.** 3D-printed/wood/plastic enclosure
- [ ] **14.** 3D-printed/laser-cut coil mount
- [ ] **15.** Centered coil design (TX outer, RX inner)

---

## 📅 Project Phases

### Phase 1: Design & Analysis
- [ ] Working and report plans
- [ ] Top-level block schematics
- [ ] Detailed circuit diagrams
- [ ] Calculations and simulations

### Phase 2: Implementation
- [ ] Build hardware in lab
- [ ] Test and debug
- [ ] Integrate all subsystems

### Phase 3: Finalization
- [ ] Re-calculate / re-design as needed
- [ ] Update documentation
- [ ] Final report (due 23/1-2026)

---

## 🏗️ System Architecture

```
                            TX PATH
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│   MCU   │───▶│ Op-Amp  │───▶│ AA Filt │───▶│ TX Coil │
│ 2kHz PWM│    │ (drive) │    │  (LPF)  │    │         │
└─────────┘    └─────────┘    └─────────┘    └────┬────┘
     ▲                                            │
     │                                       [Metal]
     │                                            │
     │                          RX PATH           ▼
┌────┴────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│ MCP3208 │◀───│ LP Filt │◀───│ Op-Amp  │◀───│ RX Coil │
│ 12b ADC │    │         │    │ (amp)   │    │         │
└────┬────┘    └─────────┘    └─────────┘    └─────────┘
     │
     ▼
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│   MCU   │───▶│   IIR   │───▶│Classify │───▶│ Display │
│   DFT   │    │ Filter  │    │         │    │ Buzzer  │
└─────────┘    └─────────┘    └─────────┘    └─────────┘
```

---

## 📊 Key Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| TX Frequency | 2 kHz | PWM generated |
| Sample Rate | 8 kHz | 4× oversampling |
| ADC Resolution | 12-bit | MCP3208 SPI |
| DFT Buffer | 64 samples | ~8ms window |
| Phase Threshold | 65° | Ferrous/non-ferrous |
| Detection Threshold | 0.2% | Minimum amplitude |

---

## 🔗 External Resources

- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [MCP3208 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/21298e.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

---

## 📝 Recent Notes

```dataview
TABLE file.mtime as "Modified"
FROM "Notes"
SORT file.mtime DESC
LIMIT 5
```

---

*Last updated: {{date}}*
