# VLF Metaldetektor Firmware

Arduino Nano (ATmega328P @ 16MHz) firmware til VLF metaldetektion.

> **Teori:** [DSP-Bible](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FDSP-Bible) | [FIIR & IIR](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FFIIR%20%26%20IIR)

## Nuværende Struktur

```
src/
├── main.c            # Al firmware i én fil
└── drivers/          # Givne drivere (modificer ikke)
    ├── I2C.c/h
    └── ssd1306.c/h
```

> **Note:** Koden er pt. samlet i `main.c` for hurtig prototyping. Se [Project_Roadmap.md](../Docs/Project_Roadmap.md) for planlagt modularisering.

## Implementeret Funktionalitet

| Funktion | Status | Beskrivelse |
|----------|--------|-------------|
| TX Signal | ✅ Færdig | 2 kHz firkantbølge via Timer0 |
| ADC Sampling | ✅ Færdig | 8 kHz auto-trigger fra Timer0 |
| DFT Beregning | ✅ Færdig | Single-bin DFT med 4× oversampling |
| Magnitude/Fase | ✅ Færdig | Beregning fra Re/Im komponenter |
| OLED Display | ✅ Færdig | Viser Re, Im, Mag, Fase |
| Debug Knap | ✅ Færdig | D4 skifter mellem DFT/Debug skærm |
| Start/Stop Knap | ❌ TODO | Krav 9a - skal implementeres |
| Kalibrering Knap | ❌ TODO | Krav 9b - skal implementeres |
| Metalklassificering | ❌ TODO | Krav 2 - ferro/non-ferro |
| IIR Filter | ❌ TODO | Krav 8c - display udglatning |

## Build

```bash
pio run              # Kompilér
pio run -t upload    # Upload til board
pio device monitor   # Seriel monitor
```

## Pin Konfiguration

| Pin | Funktion | Retning |
|-----|----------|---------|
| 9 (PB1) | TX signal output (2kHz) | OUTPUT |
| A0 (PC0) | RX signal input (ADC) | INPUT |
| A4 (PC4) | I2C SDA (OLED) | I/O |
| A5 (PC5) | I2C SCL (OLED) | OUTPUT |
| D4 (PD4) | Debug knap | INPUT (pull-up) |
| D2 (PD2) | Start/Stop knap (TODO) | INPUT (pull-up) |
| D3 (PD3) | Kalibrering knap (TODO) | INPUT (pull-up) |

## Signalflow

```
Timer0 (8kHz) ──┬──> TX Pin Toggle (hver 2. interrupt = 2kHz)
                │
                └──> ADC Auto-Trigger
                          │
                          ▼
                    ADC ISR (8kHz)
                          │
                          ▼
                    DFT Akkumulering (64 samples)
                          │
                          ▼
                    Hovedløkke: DFT_Calc() → Display
```

## Konfigurationsparametre

```c
#define F_SAMPLE 8000     // Sample frekvens (Hz)
#define F_SIGNAL 2000     // TX/RX signal frekvens (Hz)
#define N 64              // Samples per DFT vindue
```

## Næste Skridt

Se TODO-kommentarer i `main.c` for detaljeret implementeringsvejledning:
- `Krav 9a`: Start/Stop knap på D2
- `Krav 9b`: Kalibrering knap på D3 + baseline lagring
- `Krav 2`: Metalklassificering baseret på faseforskel
- `Krav 8c`: IIR lavpas filter til stabil visning

## Relaterede Dokumenter

- [Project_Roadmap.md](../Docs/Project_Roadmap.md) - Fuld kravanalyse og status
- [Code_Review.md](../Docs/Guides/Code_Review.md) - Kodegennemgang

#firmware #arduino #dft #metaldetektor
