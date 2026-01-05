# 🔧 KiCad Schematics & PCB

Hardware design files for the VLF Metal Detector.

> 🚀 **Open Project:** [Metaldetector.kicad_pro](file:///C:/Users/Mads2/34621-EM-Sensors-DSP/KiCad/Metaldetector/Metaldetector.kicad_pro)

> 📚 **Theory:** [Op-Amp Design](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLTspice%20%26%20Kicad%2F02%20-%20Two-Stage%20CMOS%20Op-Amp) | [Inductance](obsidian://open?vault=Obsidian&file=Courses%2FElectromagnetics%2FFormulas%2FL23%20-%20Magnetostatics%20II)

## Structure

```
KiCad/
├── schematics/      # .kicad_sch schematic files
│   ├── metal_detector.kicad_pro
│   ├── power_supply.kicad_sch
│   ├── tx_driver.kicad_sch
│   ├── rx_amplifier.kicad_sch
│   ├── microcontroller.kicad_sch
│   └── display_interface.kicad_sch
├── pcb/             # .kicad_pcb board files
├── footprints/      # Custom .kicad_mod footprints
├── symbols/         # Custom .kicad_sym symbols
├── gerbers/         # Manufacturing output files
└── bom/             # Bill of materials
```

## Schematic Blocks

### 1. Power Supply
- 9V battery input
- Voltage regulation (5V for MCU, 3.3V optional)
- Power filtering
- Battery monitoring (optional)

### 2. TX Coil Driver
- PWM input from Arduino (Pin 9)
- Power transistor/MOSFET driver
- Current limiting
- Coil connection (JST or screw terminal)

### 3. RX Amplifier
- High-impedance input stage
- Gain stages (op-amp)
- Bandpass filtering around 2kHz
- Output scaling to 0-5V for ADC

### 4. Microcontroller Section
- ATmega328P (DIP-28 socket for removability)
- Crystal oscillator (16MHz)
- Reset circuit
- ISP header for programming
- Decoupling capacitors

### 5. MCP3208 ADC
- SPI connections
- Reference voltage
- Anti-aliasing filter

### 6. Display & UI
- SSD1306 OLED header (I2C)
- Button inputs (calibrate, sound toggle)
- Buzzer output
- Status LEDs (optional)

## Component Selection

| Component | Part Number | Package | Notes |
|-----------|-------------|---------|-------|
| MCU | ATmega328P-PU | DIP-28 | Socketed! |
| Op-Amp | LM358 / TL072 | DIP-8 | Dual |
| ADC | MCP3208 | DIP-16 | 12-bit SPI |
| Regulator | 7805 | TO-220 | 5V 1A |
| Crystal | 16MHz | HC-49 | 22pF caps |

## Design Rules (2-layer PCB)

| Parameter | Value |
|-----------|-------|
| Min trace width | 0.25mm (10mil) |
| Min spacing | 0.25mm (10mil) |
| Via diameter | 0.8mm |
| Via drill | 0.4mm |
| Copper weight | 1oz |

## Checklist

- [ ] Schematic capture complete
- [ ] ERC (Electrical Rules Check) passed
- [ ] Footprints assigned
- [ ] PCB layout complete
- [ ] DRC (Design Rules Check) passed
- [ ] Gerbers generated
- [ ] BOM exported

## Pin Mapping (ATmega328P / Arduino Uno)

| Function | Arduino Pin | ATmega Pin | Port |
|----------|-------------|------------|------|
| PWM Out | 9 | PB1 (15) | PORTB |
| SPI CS | 10 | PB2 (16) | PORTB |
| SPI MOSI | 11 | PB3 (17) | PORTB |
| SPI MISO | 12 | PB4 (18) | PORTB |
| SPI SCK | 13 | PB5 (19) | PORTB |
| I2C SDA | A4 | PC4 (27) | PORTC |
| I2C SCL | A5 | PC5 (28) | PORTC |
| Button 1 | 2 | PD2 (4) | PORTD |
| Button 2 | 3 | PD3 (5) | PORTD |
| Buzzer | 8 | PB0 (14) | PORTB |

## Related Documents
- [[Literature/kravspecifikation.pdf|Requirements Specification]]
- [[Literature/Coil_Basics.pdf|Coil Design]]

## Tags
#kicad #pcb #hardware #schematic
