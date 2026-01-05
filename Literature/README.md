# 📚 Literature

Course materials, datasheets, and reference documents.

## Course Documents

| Document | Description |
|----------|-------------|
| [[Course_intro.pdf]] | Course introduction and overview |
| [[kravspecifikation.pdf]] | Requirements specification (Danish) |
| [[arbejdsplan.pdf]] | Work plan template (Danish) |
| [[pseudocode_stm_MD.pdf]] | Pseudocode for state machine |

## Theory & Background

### Signal Processing
| Document | Topics Covered |
|----------|----------------|
| [[The_Discrete_Fourier_Transform.pdf]] | DFT theory, implementation |
| [[Filter_lektion.pdf]] | Filter design lecture |
| [[filters_v3_III_v1.pdf]] | Advanced filter theory |

### Circuit Theory
| Document | Topics Covered |
|----------|----------------|
| [[RLC_Circuits_and_Resonance.pdf]] | Resonance, Q-factor |
| [[Lesson_3_RLC_circuits__resonance.pdf]] | RLC circuit analysis |
| [[Transients_and_Oscillations_in_RLC_Circuitsv2m.pdf]] | Transient response |
| [[Coil_Basics.pdf]] | Inductor/coil design |

### Electronics
| Document | Topics Covered |
|----------|----------------|
| [[op_amps_everyone.pdf]] | Op-amp circuits, design |
| [[duracell_9volt.pdf]] | 9V battery specifications |

### Metal Detector Specific
| Document | Topics Covered |
|----------|----------------|
| [[Advanced_Instrumentation_for_Polyharmonic_Metal_Detectors.pdf]] | Advanced MD techniques |

## Reading Order (Suggested)

1. **Week 1 - Fundamentals**
   - [ ] Course_intro.pdf
   - [ ] kravspecifikation.pdf
   - [ ] Coil_Basics.pdf
   - [ ] RLC_Circuits_and_Resonance.pdf

2. **Week 2 - Signal Processing**
   - [ ] The_Discrete_Fourier_Transform.pdf
   - [ ] Filter_lektion.pdf
   - [ ] op_amps_everyone.pdf

3. **Week 3 - Integration**
   - [ ] pseudocode_stm_MD.pdf
   - [ ] Advanced_Instrumentation_for_Polyharmonic_Metal_Detectors.pdf

## Key Concepts by Topic

### DFT (Discrete Fourier Transform)
- Single-bin DFT for 2kHz detection
- 4× oversampling optimization: `cos = {1,0,-1,0}`, `sin = {0,1,0,-1}`
- No trig functions needed!
- See: [[The_Discrete_Fourier_Transform.pdf]]

### RLC Resonance
- Resonant frequency: $f_0 = \frac{1}{2\pi\sqrt{LC}}$
- Quality factor: $Q = \frac{f_0}{BW}$
- Phase shift indicates metal type
- See: [[RLC_Circuits_and_Resonance.pdf]]

### Coil Design
- TX coil: Generate 2kHz magnetic field
- RX coil: Pick up reflected signal
- Mutual inductance and coupling
- See: [[Coil_Basics.pdf]]

### Phase Detection
- Ferrous metals: Phase < 65° (high permeability)
- Non-ferrous metals: Phase > 65° (eddy currents)
- Amplitude indicates proximity
- See: [[Course_intro.pdf]]

## External Resources

### Datasheets (to download)
- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)
- [MCP3208 Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/21298e.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- [LM358 Datasheet](https://www.ti.com/lit/ds/symlink/lm358.pdf)

## Notes Template

When reading a document, use [[Templates/Literature Note]] to create notes.

## Tags
#literature #reference #theory #datasheets
