# KiCad Skemaer & PCB

Hardware designfiler til VLF Metaldetektoren.

> **Teori:** [Op-Amp Design](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLTspice%20%26%20Kicad%2F02%20-%20Two-Stage%20CMOS%20Op-Amp) | [Induktans](obsidian://open?vault=Obsidian&file=Courses%2FElectromagnetics%2FFormulas%2FL23%20-%20Magnetostatics%20II)

## Status

| Del | Status | Noter |
|-----|--------|-------|
| TX H-bro Driver | ✅ Designet | QSPICE valideret, ~90% effektivitet |
| RX Forstærker | ⏳ Afventer | Design påbegyndt |
| Strømforsyning | ⏳ Afventer | 9V → 5V regulering |
| MCU Sektion | ⏳ Afventer | Arduino Nano breakout |

## Kredsløbsblokke

### 1. Strømforsyning
- 9V batteri input
- Spændingsregulering (5V til MCU)
- Power filtrering
- Batteri monitorering (valgfri)

### 2. TX H-bro Driver
- PWM input fra Arduino (Pin 9)
- **P-kanal MOSFET:** IRF5305PbF (2 stk)
- **N-kanal MOSFET:** IRL530 (2 stk)
- Gate drivere til niveau-skift
- Spoletilslutning (skrueterminal)
- **Effektivitet:** ~90% (QSPICE verificeret)

### 3. RX Forstærker
- Høj-impedans indgangstrin
- Forstærkertrin (op-amp)
- Båndpasfilter omkring 2kHz
- Output skalering til 0-5V for ADC

### 4. Microcontroller Sektion
- Arduino Nano (ATmega328P)
- Intern 10-bit ADC (ingen ekstern ADC nødvendig)
- Timer0 auto-trigger til synkroniseret sampling

### 5. Display & UI
- SSD1306 OLED header (I2C)
- Knapinput (Start/Stop, Kalibrering, Debug)
- Summer output (valgfri)

## Komponentvalg

| Komponent | Delnummer | Pakke | Noter |
|-----------|-----------|-------|-------|
| MCU | Arduino Nano | - | ATmega328P @ 16MHz |
| P-MOSFET | IRF5305PbF | TO-220 | H-bro højside |
| N-MOSFET | IRL530 | TO-220 | H-bro lavside |
| Op-Amp | LM358 / TL072 | DIP-8 | Dual |
| Regulator | 7805 | TO-220 | 5V 1A |
| OLED | SSD1306 | - | 128×64, I2C |

## Pin Mapping (Arduino Nano / ATmega328P)

| Funktion | Arduino Pin | ATmega Pin | Port |
|----------|-------------|------------|------|
| TX Output | 9 | PB1 (15) | PORTB |
| RX Input (ADC) | A0 | PC0 (23) | PORTC |
| I2C SDA | A4 | PC4 (27) | PORTC |
| I2C SCL | A5 | PC5 (28) | PORTC |
| Debug Knap | 4 | PD4 (6) | PORTD |
| Start/Stop | 2 | PD2 (4) | PORTD |
| Kalibrering | 3 | PD3 (5) | PORTD |

## Design Regler (2-lags PCB)

| Parameter | Værdi |
|-----------|-------|
| Min sporbredde | 0.25mm (10mil) |
| Min afstand | 0.25mm (10mil) |
| Via diameter | 0.8mm |
| Via boring | 0.4mm |
| Kobbertykkelse | 1oz |

## Tjekliste

- [x] H-bro design færdigt
- [x] QSPICE simulering bekræftet
- [ ] Skematisk capture komplet
- [ ] ERC (Electrical Rules Check) bestået
- [ ] Footprints tildelt
- [ ] PCB layout komplet
- [ ] DRC (Design Rules Check) bestået
- [ ] Gerbers genereret
- [ ] BOM eksporteret

## Relaterede Dokumenter

- [TX Driver Design.md](../Docs/Theory/TX%20Driver%20Design.md) - H-bro designanalyse
- [Power Budget Analysis.md](../Docs/Theory/Power%20Budget%20Analysis.md) - Strømforbrugsanalyse
- [[../Literature/kravspecifikation.pdf|Kravspecifikation]]

#kicad #pcb #hardware #skema
