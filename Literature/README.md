# Litteratur

Kursusmaterialer, datablade og referencedokumenter til VLF Metaldetektor projektet.

## Kursusdokumenter

| Dokument | Beskrivelse |
|----------|-------------|
| [[Course_intro.pdf]] | Kursusintroduktion og overblik |
| [[kravspecifikation.pdf]] | Kravspecifikation (16 krav) |
| [[arbejdsplan.pdf]] | Arbejdsplan skabelon |
| [[pseudocode_stm_MD.pdf]] | Pseudokode til tilstandsmaskine |

## Teori & Baggrund

### Signalbehandling
| Dokument | Emner |
|----------|-------|
| [[The_Discrete_Fourier_Transform.pdf]] | DFT teori, implementering |
| [[Filter_lektion.pdf]] | Filterdesign forelæsning |
| [[filters_v3_III_v1.pdf]] | Avanceret filterteori |

### Kredsløbsteori
| Dokument | Emner |
|----------|-------|
| [[RLC_Circuits_and_Resonance.pdf]] | Resonans, Q-faktor |
| [[Lesson_3_RLC_circuits__resonance.pdf]] | RLC kredsløbsanalyse |
| [[Transients_and_Oscillations_in_RLC_Circuitsv2m.pdf]] | Transient respons |
| [[Coil_Basics.pdf]] | Spoledesign |

### Elektronik
| Dokument | Emner |
|----------|-------|
| [[op_amps_everyone.pdf]] | Op-amp kredsløb, design |
| [[duracell_9volt.pdf]] | 9V batteri specifikationer |

### Metaldetektor Specifikt
| Dokument | Emner |
|----------|-------|
| [[Advanced_Instrumentation_for_Polyharmonic_Metal_Detectors.pdf]] | Avancerede MD teknikker |

## Læserækkefølge (Anbefalet)

1. **Uge 1 - Grundlæggende**
   - [ ] Course_intro.pdf
   - [ ] kravspecifikation.pdf
   - [ ] Coil_Basics.pdf
   - [ ] RLC_Circuits_and_Resonance.pdf

2. **Uge 2 - Signalbehandling**
   - [ ] The_Discrete_Fourier_Transform.pdf
   - [ ] Filter_lektion.pdf
   - [ ] op_amps_everyone.pdf

3. **Uge 3 - Integration**
   - [ ] pseudocode_stm_MD.pdf
   - [ ] Advanced_Instrumentation_for_Polyharmonic_Metal_Detectors.pdf

## Nøglebegreber

### DFT (Diskret Fourier Transformation)
- Single-bin DFT til 2kHz detektion
- 4× oversampling optimering: `cos = {1,0,-1,0}`, `sin = {0,1,0,-1}`
- Ingen trigonometriske funktioner nødvendige!
- Se: [[The_Discrete_Fourier_Transform.pdf]]

### RLC Resonans
- Resonansfrekvens: $f_0 = \frac{1}{2\pi\sqrt{LC}}$
- Kvalitetsfaktor: $Q = \frac{f_0}{BW}$
- Faseforskel indikerer metaltype
- Se: [[RLC_Circuits_and_Resonance.pdf]]

### Spoledesign
- TX spole: Generér 2kHz magnetfelt (Ø200mm, 20 vindinger)
- RX spole: Opfang reflekteret signal (Ø80mm, 200 vindinger)
- Bucking spole: Annullér direkte kobling (Ø120mm, 20 vindinger)
- Se: [[Coil_Basics.pdf]], [Coil Design.md](../Docs/Theory/Coil%20Design.md)

### Fasedetektion
- Ferromagnetiske metaller: Fase < 65° (høj permeabilitet)
- Ikke-ferromagnetiske metaller: Fase > 65° (hvirvelstrømme)
- Amplitude indikerer nærhed
- Se: [[Course_intro.pdf]]

## Eksterne Ressourcer

### Datablade (til download)
- [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf) - Microcontroller
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf) - OLED display
- [IRF5305PbF Datasheet](https://www.infineon.com/dgdl/irf5305pbf.pdf) - P-kanal MOSFET (H-bro)
- [IRL530 Datasheet](https://www.infineon.com/dgdl/irl530.pdf) - N-kanal MOSFET (H-bro)
- [LM358 Datasheet](https://www.ti.com/lit/ds/symlink/lm358.pdf) - Op-amp

## Relaterede Dokumenter

- [Project_Roadmap.md](../Docs/Project_Roadmap.md) - Projektstatus og kravanalyse
- [Theory References.md](../Docs/Theory/Theory%20References.md) - Teori krydsreferencer

#litteratur #reference #teori #datablade
