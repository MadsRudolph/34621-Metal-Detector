# VLF Metaldetektor Firmware

Arduino Nano (ATmega328P @ 16MHz) firmware til VLF metaldetektion.

> **Teori:** [DSP-Bible](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FDSP-Bible) | [FIIR & IIR](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FFIIR%20%26%20IIR)

## Nuværende Struktur

```
src/
├── main.c            # Hovedprogram, initialisering, hovedløkke
├── adc.c             # ADC sampling og auto-trigger
├── button.c          # Knaphåndtering med debounce
├── buzzer.c          # Buzzer feedback (beeps ved detektion)
├── capture.c         # MATLAB serial capture interface
├── detection.c       # Metal klassificering (ferro/non-ferro)
├── dft.c             # DFT beregning og akkumulering
├── display.c         # OLED display med grafisk HUD og ikoner
├── filter.c          # IIR filter til udglatning
├── jingle.c          # Startup-melodi
├── timer.c           # Timer0/Timer1 konfiguration
│
├── include/          # Header filer
│   ├── config.h      # Globale konstanter og konfiguration
│   ├── adc.h, button.h, buzzer.h, capture.h
│   ├── detection.h, dft.h, display.h, filter.h
│   ├── jingle.h, timer.h
│
└── drivers/          # Hardware drivere (modificer ikke)
    ├── I2C.c/h       # TWI master driver
    ├── ssd1306.c/h   # OLED display driver
    └── data.h        # Font og ikon data (PROGMEM)
```

## Implementeret Funktionalitet

| Funktion | Status | Beskrivelse |
|----------|--------|-------------|
| TX Signal | ✅ Færdig | 2 kHz firkantbølge via Timer0 |
| ADC Sampling | ✅ Færdig | 8 kHz auto-trigger fra Timer0 |
| DFT Beregning | ✅ Færdig | Single-bin DFT med 4× oversampling |
| Magnitude/Fase | ✅ Færdig | Beregning fra Re/Im komponenter |
| OLED Display | ✅ Færdig | Grafisk HUD med ikoner og progress bar |
| Debug Knap | ✅ Færdig | D4 skifter mellem DFT/Debug skærm |
| Start/Stop Knap | ✅ Færdig | D2 toggle detektor on/off |
| Kalibrering Knap | ✅ Færdig | D3 gemmer baseline (luft-aflæsning) |
| Metalklassificering | ✅ Færdig | Ferro/non-ferro baseret på fase |
| IIR Filter | ✅ Færdig | Udglatning af display værdier |
| Buzzer Feedback | ✅ Færdig | Bip ved metal detektion |
| Startup Jingle | ✅ Færdig | Melodi ved opstart |
| Splash Screen | ✅ Færdig | Logo ved opstart |

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
| 11 (PB3) | Buzzer output (PWM tone) | OUTPUT |
| A0 (PC0) | RX signal input (ADC) | INPUT |
| A4 (PC4) | I2C SDA (OLED) | I/O |
| A5 (PC5) | I2C SCL (OLED) | OUTPUT |
| D2 (PD2) | Start/Stop knap | INPUT (pull-up) |
| D3 (PD3) | Kalibrering knap | INPUT (pull-up) |
| D4 (PD4) | Debug knap | INPUT (pull-up) |

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
                    IIR Filter → Klassificering → Display + Buzzer
```

## Konfigurationsparametre

```c
// Fra include/config.h
#define F_SAMPLE 8000     // Sample frekvens (Hz)
#define F_SIGNAL 2000     // TX/RX signal frekvens (Hz)
#define N 64              // Samples per DFT vindue
#define IIR_ALPHA 0.15f   // Filter udglatning
#define FERRO_THRESHOLD -10   // Fase tærskel for ferro (grader)
#define DETECT_THRESHOLD 5    // Min. magnitude ændring for detektion
```

## Moduler

| Modul | Beskrivelse |
|-------|-------------|
| `timer.c` | Timer0 (8kHz interrupt, TX toggle) og Timer1 (buzzer PWM) |
| `adc.c` | ADC auto-trigger setup og sample håndtering |
| `dft.c` | Optimeret single-bin DFT (kun bin k=16 ved 2kHz) |
| `filter.c` | IIR lavpas filter for stabil visning |
| `detection.c` | Kalibrering og metal klassificering (ferro/non-ferro) |
| `display.c` | Grafisk HUD med ikoner, progress bar, splash screen |
| `button.c` | Debounced knaphåndtering for D2, D3, D4 |
| `buzzer.c` | Tone-generering via Timer1 PWM |
| `jingle.c` | Startup-melodi sekvens |
| `capture.c` | MATLAB serial interface til DFT verifikation |

## Relaterede Dokumenter

- [Project_Roadmap.md](../Docs/Project_Roadmap.md) - Fuld kravanalyse og status
- [Code_Review.md](../Docs/Guides/Code_Review.md) - Kodegennemgang

#firmware #arduino #dft #metaldetektor
