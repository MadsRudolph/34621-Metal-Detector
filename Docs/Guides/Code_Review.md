# Metaldetektor Firmware Kodegennemgang

> [!abstract] Dokumentformål
> Dette dokument giver en teknisk gennemgang af VLF metaldetektor firmwaren.
> **Nuværende Status:** Fuldt funktionel og modulær version med metaldetektion, klassificering, kalibrering, grafisk HUD, buzzer feedback og startup jingle.
>
> **Mål:** ATmega328P (Arduino Nano)
> **Kursus:** DTU 34621 - Indlejrede Systemer

---

## Indholdsfortegnelse

1. [[#1. Systemoversigt]]
2. [[#2. Kodestruktur]]
3. [[#3. Hardware Konfiguration]]
4. [[#4. Signalbehandling]]
5. [[#5. Metal Klassificering]]
6. [[#6. Buzzer og Jingle]]
7. [[#7. Kompilering og Upload]]
8. [[#8. Brugergrænseflade]]
9. [[#9. Fejlfinding]]

---

## 1. Systemoversigt

### 1.1 Nuværende Arkitektur

Firmwaren er en fuldt funktionel og modulær metaldetektor med følgende funktioner:
- TX signalgenerering (2 kHz firkantbølge)
- RX signal sampling og DFT analyse
- IIR filtrering for stabil visning
- Metal klassificering (ferro/non-ferro baseret på fase)
- Kalibrering med baseline lagring
- Start/Stop kontrol
- Debug display mode
- Grafisk HUD med ikoner og progress bar
- Buzzer feedback ved metal detektion
- Startup jingle ved opstart
- Splash screen ved boot

```
Code/src/
├── main.c              # Hovedprogram, initialisering, hovedløkke
├── adc.c               # ADC sampling og auto-trigger
├── button.c            # Knaphåndtering med debounce
├── buzzer.c            # Buzzer feedback (beeps ved detektion)
├── capture.c           # MATLAB serial capture interface
├── detection.c         # Metal klassificering (ferro/non-ferro)
├── dft.c               # DFT beregning og akkumulering
├── display.c           # OLED display med grafisk HUD og ikoner
├── filter.c            # IIR filter til udglatning
├── jingle.c            # Startup-melodi
├── timer.c             # Timer0/Timer1 konfiguration
│
├── include/            # Header filer
│   ├── config.h        # Globale konstanter og konfiguration
│   ├── adc.h, button.h, buzzer.h, capture.h
│   ├── detection.h, dft.h, display.h, filter.h
│   └── jingle.h, timer.h
│
└── drivers/            # Hardware drivere
    ├── I2C.c, I2C.h    # TWI master driver
    ├── ssd1306.c, .h   # OLED display driver
    └── data.h          # Font og ikon data (PROGMEM)
```

### 1.2 Signalflow

```mermaid
flowchart TB
    T0[Timer0<br>8kHz] --> TX[TX Spole<br>Pin 9<br>2kHz]
    TX -.->|Felt| M((Metal))
    M -.->|Hvirvel| RX[RX Spole]
    RX --> ADC[ADC<br>Pin A0]
    ADC --> DFT[DFT<br>64 samples]
    DFT --> IIR[IIR Filter]
    IIR --> CLASS[Klassificering]
    CLASS --> OLED[Display]
```

**Signalkæde:** Timer0 (8kHz) → TX Toggle (2kHz) → Metal → RX Spole → ADC → DFT → IIR Filter → Klassificering → Display

### 1.3 Nøglespecifikationer

| Parameter | Værdi | Noter |
|-----------|-------|-------|
| TX Frekvens | 2 kHz | VLF område |
| Samplerate | 8 kHz | 4 samples per TX cyklus |
| ADC Trigger | Auto (ADATE) | Timer0 Compare Match A |
| DFT Vindue | 64 samples | 8 ms data |
| TX Cykler per Vindue | 16 | 64 / 4 = 16 |
| IIR Alpha | 0.15 | Udglætningsfaktor |
| I2C Clock | ~400 kHz | Fast mode |
| ADC Opløsning | 10 bits | 0-1023 område |
| ADC Prescaler | 64 | 250 kHz ADC clock |

### 1.4 Pin Konfiguration

| Pin | AVR Port | Funktion | Noter |
|-----|----------|----------|-------|
| 9 | PB1 | TX signal | 2 kHz firkantbølge |
| 11 | PB3 | Buzzer output | PWM tone via Timer1 |
| A0 | PC0/ADC0 | RX signal indgang | 10-bit ADC |
| A4 | PC4 | I2C SDA | OLED display |
| A5 | PC5 | I2C SCL | OLED display |
| D2 | PD2 | Start/Stop knap | Toggle detektor on/off |
| D3 | PD3 | Kalibrering knap | Gem baseline |
| D4 | PD4 | Debug knap | Skift mellem DFT og debug skærm |

---

## 2. Kodestruktur

### 2.1 Modulær Arkitektur

Firmwaren er opdelt i 11 separate moduler med tilhørende header-filer i `include/` mappen:

| Modul | Ansvar |
|-------|--------|
| `main.c` | Hovedløkke, initialisering, tilstandshåndtering |
| `timer.c` | Timer0 (8kHz interrupt, TX toggle) og Timer1 (buzzer PWM) |
| `adc.c` | ADC auto-trigger setup og initialisering |
| `dft.c` | Single-bin DFT akkumulering og beregning |
| `filter.c` | IIR lavpas filter for udglatning |
| `detection.c` | Kalibrering og metal klassificering |
| `display.c` | Grafisk HUD, ikoner, progress bar, splash screen |
| `button.c` | Debounced knaphåndtering |
| `buzzer.c` | Tone-generering via Timer1 PWM |
| `jingle.c` | Startup-melodi sekvens |
| `capture.c` | MATLAB serial interface |

#### Konfiguration (include/config.h)

```c
#define F_SAMPLE 8000         // Sample frekvens (Hz)
#define F_SIGNAL 2000         // TX/RX signal frekvens (Hz)
#define N 64                  // Samples per DFT vindue
#define ADC_OFFSET 512        // DC offset

/* Metal Klassificering */
#define FERRO_THRESHOLD -10   // Fase tærskel (grader)
#define DETECT_THRESHOLD 5    // Min. magnitude ændring

/* IIR Filter */
#define IIR_ALPHA 0.15f       // Udglatningsfaktor
```

> [!WARNING] Signed Typer Påkrævet
> `Re` og `Im` SKAL være `int32_t` (signed), ikke `uint32_t`.
> DFT'en subtraherer værdier, så negative resultater er forventede.

#### Timer0 Initialisering

```c
void timer0_init(void) {
    DDRB |= (1 << PB1);           /* Pin 9 udgang */
    TCCR0A = (1 << WGM01);        /* CTC mode */
    TCCR0B = (1 << CS01);         /* Prescaler 8 */
    OCR0A = 249;                  /* 8 kHz interrupt */
    TIMSK0 = (1 << OCIE0A);
}
```

**Frekvensberegning:**
- f_interrupt = 16 MHz / (8 × 250) = 8 kHz
- TX toggles hver 2. interrupt = 4 kHz toggle rate = 2 kHz firkantbølge

#### ADC Initialisering

```c
void adc_init(void) {
    ADMUX = (1 << REFS0);         /* AVCC reference */
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADATE)
           | (1 << ADPS2) | (1 << ADPS1);  /* Prescaler 64 */
    ADCSRB = (1 << ADTS1) | (1 << ADTS0);  /* Timer0 trigger */
    ADCSRA |= (1 << ADSC);        /* Start */
}
```

> [!TIP] Auto-Trigger Mode
> Ved brug af `ADATE` med `ADTS1:0 = 011` starter ADC automatisk en ny
> konvertering ved hver Timer0 Compare Match A event. Ingen manuel triggering nødvendig.

#### Button Initialisering

```c
void button_init(void) {
    DDRD &= ~(1 << PD4);          /* D4 Debug - input */
    PORTD |= (1 << PD4);          /* Pull-up aktiveret */
    DDRD &= ~(1 << PD3);          /* D3 Calibrate - input */
    PORTD |= (1 << PD3);          /* Pull-up aktiveret */
    DDRD &= ~(1 << PD2);          /* D2 Start/Stop - input */
    PORTD |= (1 << PD2);          /* Pull-up aktiveret */
}
```

Alle tre knapper bruger interne pull-up modstande. Knapperne er active-low (forbundet til GND ved tryk).

#### Timer0 ISR (TX Generering)

```c
ISR(TIMER0_COMPA_vect) {
    if (!detector_running) {
        PORTB &= ~(1 << PB1);     /* TX lav når stoppet */
        tx_count = 0;
        return;
    }

    if (++tx_count >= 2) {
        PORTB ^= (1 << PB1);      /* Toggle TX */
        tx_count = 0;
        if (PORTB & (1 << PB1)) {
            sync_flag = 1;        /* Synkroniser DFT med TX rising edge */
        }
    }
}
```

ISR'en tjekker først om detektoren kører. Hvis ikke, holdes TX lav for at spare strøm og undgå støj.

#### ADC ISR (DFT Akkumulering)

```c
ISR(ADC_vect) {
    adc_raw = ADC;
    if (sync_flag && detector_running) {
        dft_sum(adc_raw);
    }
}
```

#### DFT Akkumulering

```c
void dft_sum(int16_t raw) {
    int16_t x = raw - ADC_OFFSET;

    switch (dft_index & 3) {
        case 0: Re += x;  break;
        case 1: Im -= x;  break;
        case 2: Re -= x;  break;
        case 3: Im += x;  break;
    }

    if (++dft_index >= N) {
        Re_buf = Re;
        Im_buf = Im;
        Re = Im = 0;
        dft_index = 0;
        dft_done = 1;
        sync_flag = 0;
    }
}
```

Optimeret single-bin DFT der udnytter at cos/sin koefficienter ved 4× oversampling kun er +1, -1 eller 0.

#### DFT Beregning

```c
void dft_calc(void) {
    double re = Re_buf, im = Im_buf;
    mag = sqrt(re*re + im*im) / N;
    phase = atan2(im, re) * 57.2957795;
}
```

> [!NOTE] Floating-Point Matematik
> Bruger standard bibliotek `sqrt()` og `atan2()` fra `<math.h>` for nøjagtig beregning.
> Kan optimeres med integer approksimationer senere hvis nødvendigt.

---

## 3. Hardware Konfiguration

### 3.1 Timer0 - TX Generering & ADC Auto-Trigger

Timer0 tjener dobbelt formål:
1. Generer 8 kHz Compare Match A event til ADC auto-triggering
2. ISR toggler TX pin hver 2. interrupt for 2 kHz signal

```mermaid
flowchart TD
    CLK[16 MHz Clock] --> PRE[Prescaler /8<br>2 MHz]
    PRE --> CNT[TCNT0 Tæller<br>0 → 249 → 0]
    CNT --> CMP{TCNT0 == OCR0A?}
    CMP -->|Ja| EVENT[Compare Match A Event]
    CMP -->|Nej| CNT

    EVENT --> ISR[TIMER0_COMPA ISR<br>Toggle TX hver 2.]
    EVENT --> ADC[ADC Auto-Trigger<br>via ADATE]

    ADC --> CONV[ADC Konvertering]
    CONV --> ADC_ISR[ADC_vect ISR<br>DFT akkumulering]
```

### 3.2 ADC Timing

| Parameter | Værdi | Beregning |
|-----------|-------|-----------|
| ADC clock | 250 kHz | 16 MHz / 64 |
| Konverteringstid | 52 µs | 13 cykler / 250 kHz |
| Sample interval | 125 µs | 8 kHz rate |
| Margin | 73 µs | 125 - 52 |

### 3.3 Timingdiagram

```
Tid (µs):      0    125   250   375   500
               │     │     │     │     │
Timer0 ISR:   [0]   [1]   [2]   [3]   [4]  (8 kHz)
TX Toggle:    Ja    Nej   Ja    Nej   Ja
TX Output:    ─────HIGH─────┐     ┌─────HIGH─────
                            └LOW─┘
ADC Sample:   S0    S1    S2    S3         (4 samples per TX cyklus)
DFT coeff:   Re+   Im-   Re-   Im+

             ◄───── 500µs (én TX cyklus, 2 kHz) ─────►
```

### 3.4 DFT Koefficient Mapping

```mermaid
flowchart LR
    subgraph "TX Cyklus 500µs"
        S0[Sample 0<br>0°] --> S1[Sample 1<br>90°]
        S1 --> S2[Sample 2<br>180°]
        S2 --> S3[Sample 3<br>270°]
    end
    
    S0 -.-> R0[Re += x]
    S1 -.-> I1[Im -= x]
    S2 -.-> R2[Re -= x]
    S3 -.-> I3[Im += x]
```

---

## 4. Signalbehandling

> [!info] DFT Algoritme Dokumentation
> Se [[../Theory/DFT Algorithm|DFT Algoritme]] for fuld matematisk baggrund.

Firmwaren bruger en optimeret single-bin DFT der udnytter 4× oversampling:
- Sampler ved 8 kHz, signal ved 2 kHz
- Cos/sin koefficienter reduceres til +1, -1, 0
- Ingen trigonometriske beregninger i ISR

### 4.1 IIR Filter

Et single-pole IIR lavpasfilter udglatter magnitude og fase for stabil visning:

```c
#define IIR_ALPHA 0.15f

void filter_update(void) {
    mag_filtered = IIR_ALPHA * mag + (1.0f - IIR_ALPHA) * mag_filtered;
    phase_filtered = IIR_ALPHA * phase + (1.0f - IIR_ALPHA) * phase_filtered;
}
```

**Karakteristik:**
- Alpha = 0.15 giver god udglætning med rimelig responstid
- Højere alpha = hurtigere respons, mere støj
- Lavere alpha = langsommere respons, mere udglætning

**Matematisk formel:**
```
y[n] = α × x[n] + (1 - α) × y[n-1]
```

### 4.2 IIR Filter Blokdiagram

```mermaid
flowchart LR
    X[x_n<br>Input] --> MUL1[× α]
    MUL1 --> ADD((+))
    ADD --> Y[y_n<br>Output]
    Y --> Z["z⁻¹<br>Delay"]
    Z --> MUL2[× 1-α]
    MUL2 --> ADD
```

---

## 5. Metal Klassificering

### 5.1 Princip

Metal klassificeres baseret på faseændring fra kalibreret baseline:
- **Ferromagnetiske metaller** (jern, stål): Negativ faseændring (fase skifter mod negative værdier)
- **Ikke-ferromagnetiske metaller** (kobber, aluminium, messing): Positiv/neutral faseændring

Dette skyldes at ferromagnetiske materialer har høj permeabilitet som forstærker det magnetiske felt, mens ikke-ferromagnetiske materialer primært skaber hvirvelstrøms-tab.

### 5.2 Klassificeringsflowchart

```mermaid
flowchart TD
    START[Ny DFT Måling] --> CAL{Kalibreret?}
    CAL -->|Nej| NONE1[METAL_NONE]
    CAL -->|Ja| DELTA[Beregn Δmag]
    DELTA --> THRESH{Δmag >= 5?}
    THRESH -->|Nej| NONE2[METAL_NONE<br>Ingen detektion]
    THRESH -->|Ja| PHASE[Beregn Δphase]
    PHASE --> FERRO{Δphase < -10°?}
    FERRO -->|Ja| FE[METAL_FERRO<br>Jern/Stål]
    FERRO -->|Nej| NONFE[METAL_NONFERRO<br>Kobber/Alu]
```

### 5.3 Detektionstærskel

Før klassificering tjekkes om magnitude-ændringen overstiger en tærskel:

```c
#define DETECT_THRESHOLD 5      /* Min. magnitude ændring for detektion */
```

Dette forhindrer falske detektioner fra støj når intet metal er tilstede.

### 5.4 Klassificeringsalgoritme

```c
void classify_metal(void) {
    if (!is_calibrated) {
        metal_type = METAL_NONE;
        metal_detected = 0;
        return;
    }

    /* Beregn magnitude ændring (absolut værdi) */
    int16_t delta_mag = (int16_t)mag_filtered - (int16_t)baseline_mag;
    if (delta_mag < 0) delta_mag = -delta_mag;

    /* Check om magnitude ændring overstiger tærskel */
    if (delta_mag < DETECT_THRESHOLD) {
        metal_detected = 0;
        metal_type = METAL_NONE;
        return;
    }

    /* Metal detekteret - klassificer baseret på fase */
    metal_detected = 1;
    int16_t delta_phase = (int16_t)phase_filtered - baseline_phase;
    if (delta_phase < FERRO_THRESHOLD) {
        metal_type = METAL_FERRO;
    } else {
        metal_type = METAL_NONFERRO;
    }
}
```

### 5.5 Kalibrering

Tryk på D3 knappen for at gemme nuværende filtrerede værdier som baseline:

```c
void calibrate(void) {
    baseline_mag = (uint16_t)mag_filtered;
    baseline_phase = (int16_t)phase_filtered;
    is_calibrated = 1;
    eeprom_save_calibration();
}
```

Kalibrering bør udføres med detektoren i luften, væk fra metal.

### 5.6 Tærskelværdier

| Parameter | Værdi | Beskrivelse |
|-----------|-------|-------------|
| FERRO_THRESHOLD | -10 | Fase delta < -10 grader = ferromagnetisk |
| DETECT_THRESHOLD | 5 | Min. magnitude delta for at registrere metal |

---

## 6. Buzzer og Jingle

### 6.1 Buzzer Feedback

Buzzeren giver audio feedback ved metal detektion og bruges til startup jingle.

**Hardware:**
- Pin 11 (PB3/OC2A) - PWM output
- Passiv buzzer (piezo eller magnetisk)

**Funktioner (buzzer.c):**

```c
void buzzer_init(void);        // Initialiser Timer1 for PWM
void buzzer_beep(uint16_t freq, uint16_t duration_ms);  // Afspil tone
void buzzer_off(void);         // Stop buzzer
```

### 6.2 Startup Jingle

Ved opstart afspilles en kort melodi via `jingle.c`:

```c
void jingle_play(void);        // Afspil startup-melodi
```

Jinglen består af en sekvens af toner der afspilles efter splash screen.

### 6.3 Metal Detektion Feedback

Når metal detekteres, bipper buzzeren for at give audio feedback:
- **Ferro metal (Fe/Stål):** Lavere tone
- **Non-ferro metal (Cu/Al):** Højere tone

Feedback hjælper brugeren med at identificere metal uden at kigge på displayet.

---

## 7. Kompilering og Upload

### 7.1 PlatformIO Konfiguration

```ini
[env:nano]
platform = atmelavr
board = nanoatmega328
framework = arduino

board_build.f_cpu = 16000000L

build_flags =
    -std=c99
    -Wall
    -Os
```

### 7.2 Build Kommandoer

```bash
# Kompiler
pio run

# Upload til Arduino Nano
pio run -t upload

# Monitor seriel output
pio device monitor
```

### 7.3 Hukommelsesforbrug

| Hukommelse | Brugt | Tilgængelig | Procent |
|------------|-------|-------------|---------|
| RAM | 518 bytes | 2048 bytes | 25.3% |
| Flash | 7324 bytes | 30720 bytes | 23.8% |

---

## 8. Brugergrænseflade

### 8.1 Knapper

| Knap | Pin | Funktion | Noter |
|------|-----|----------|-------|
| D2 | PD2 | Start/Stop | Toggle detektor on/off |
| D3 | PD3 | Kalibrering | Gem baseline (kun når kørende) |
| D4 | PD4 | Debug | Skift mellem DFT og debug skærm |

Alle knapper har 50ms debounce delay og bruger falling-edge detection.

### 8.2 Tilstandsdiagram

```mermaid
stateDiagram-v2
    [*] --> Stoppet: Boot
    Stoppet --> Kørende: D2 Tryk
    Kørende --> Stoppet: D2 Tryk
    
    state Kørende {
        [*] --> DFT_Skærm
        DFT_Skærm --> Debug_Skærm: D4 Tryk
        Debug_Skærm --> DFT_Skærm: D4 Tryk
        DFT_Skærm --> Kalibrering: D3 Tryk
        Debug_Skærm --> Kalibrering: D3 Tryk
        Kalibrering --> DFT_Skærm: Auto
    }
```

### 8.3 Knaphåndtering i Main Loop

```c
/* Start/Stop knap (D2) */
uint8_t btn2 = (PIND >> PD2) & 1;
if (btn2_prev && !btn2) {           /* Falling edge */
    _delay_ms(50);                   /* Debounce */
    if (detector_running) {
        detector_stop();
        display_stopped();
    } else {
        detector_start();
    }
}
btn2_prev = btn2;

/* Kalibrering knap (D3) */
uint8_t btn3 = (PIND >> PD3) & 1;
if (btn3_prev && !btn3 && detector_running) {
    _delay_ms(50);
    calibrate();
    /* Vis bekræftelse */
}
btn3_prev = btn3;

/* Debug knap (D4) */
uint8_t btn = (PIND >> PD4) & 1;
if (btn_prev && !btn && detector_running) {
    _delay_ms(50);
    show_debug = !show_debug;
}
btn_prev = btn;
```

### 8.4 Display Skærme

Displayet bruger en grafisk HUD med ikoner og progress bar.

**Splash Screen (ved opstart):**
- Vises i ~2 sekunder ved boot
- Projekt logo/navn
- Efterfulgt af startup jingle

**Hovedskærm (Grafisk HUD):**
- Ikon for metal type (ferro/non-ferro/ingen)
- Progress bar for signal styrke
- Numeriske værdier for magnitude og fase
- Kalibreringsindikator

**Skærmelementer:**
```
┌─────────────────────────┐
│  [IKON]  METAL TYPE     │
│                         │
│  MAG: ████████░░  120   │
│  PHA: -45°              │
│                         │
│  [CAL]  Status tekst    │
└─────────────────────────┘
```

**Metal Type Ikoner:**
- Intet metal: Tom cirkel eller "---"
- Ferro (Fe/Stål): Magnet-ikon
- Non-ferro (Cu/Al): Ring-ikon

**Stoppet Skærm:**
- Viser "STOPPET" med instruktion
- Tryk D2 for at starte

**Debug Skærm (D4 toggle):**
- Rå ADC værdi (0-1023)
- Ufiltreret vs filtreret magnitude
- Delta fra baseline

---

## 9. Fejlfinding

### Intet TX Signal på Pin 9

1. Tjek at detektoren er startet (tryk D2)
2. Verificer at `DDRB |= (1 << PB1)` er udført i `timer0_init()`
3. Tjek at `sei()` kaldes i `main()` (aktiverer interrupts)
4. Mål med oscilloskop (bør være 2kHz firkantbølge)

### ADC Sampler Ikke

1. Verificer at `sei()` kaldes (aktiverer interrupts)
2. Tjek at detektoren kører (`detector_running = 1`)
3. Verificer at ADCSRA har ADEN, ADIE og ADATE bits sat
4. Verificer at ADCSRB har ADTS1:0 = 011 (Timer0 Compare Match A trigger)

### Display Virker Ikke

1. Tjek I2C forbindelser (A4=SDA, A5=SCL)
2. Verificer at `I2C_Init()` kaldes før `InitializeDisplay()`
3. Prøv anden I2C adresse (nogle moduler bruger 0x7A i stedet for 0x78)

### Kalibrering Indlæses Ikke Ved Opstart

1. Verificer at EEPROM ikke er blevet slettet
2. Tjek at magic value er korrekt (0xCAFE på adresse 0-1)
3. Kalibrer igen med D3 knappen for at gemme ny kalibrering

### Metal Detekteres Ikke

1. Sørg for at detektoren er kalibreret (display viser ikke "Tryk D3 for CAL")
2. Tjek at DETECT_THRESHOLD (5) ikke er for høj til dit setup
3. Verificer at RX spole er korrekt forbundet til A0
4. Tjek at TX spole sender signal (oscilloskop på pin 9)

### Forkert Metal Klassificering

1. Kalibrer igen i et område uden metal
2. Tjek at FERRO_THRESHOLD (-10) er passende
3. Verificer at fasen er stabil (brug debug skærm)

---

## 10. Fremtidige Forbedringer

Følgende funktioner kan tilføjes senere:

- [x] ~~Buzzer output med Timer1 PWM for audio feedback~~ ✅ Implementeret
- [x] ~~Grafisk HUD med ikoner~~ ✅ Implementeret
- [x] ~~Startup jingle~~ ✅ Implementeret
- [ ] EEPROM persistens af kalibrering
- [ ] Seriel debug output for PC-baseret analyse
- [ ] Justerbare tærskelværdier via menu system
- [ ] Batteriniveau visning
- [ ] Automatisk gain kontrol
- [ ] Flere frekvenser for bedre diskriminering

---

## Dokumentrevisionshistorik

| Version | Dato | Ændringer |
|---------|------|-----------|
| 1.0 | 2025-01 | Første omfattende gennemgang (ATmega2560) |
| 2.0 | 2025-01 | Migreret til Arduino Nano (ATmega328P) |
| 2.1 | 2025-01 | Opdateret til at afspejle ADC auto-trigger mode (ADATE) |
| 2.2 | 2025-01 | Ændret til floating-point sqrt()/atan2() |
| 3.0 | 2025-01 | Forenklet til minimal testversion (enkelt main.c) |
| 4.0 | 2025-01 | Fuldt funktionel version med metal klassificering, IIR filter, Start/Stop kontrol |
| 5.0 | 2026-01 | Modularisering: 11 moduler + include/ mappe. Tilføjet buzzer, jingle, grafisk HUD med ikoner og splash screen |

---

*Dokument genereret til DTU 34621 Metaldetektor Projekt*
*Hold: Mads Rudolph, Andreas Skaaning, Jonas Beck & Sigurd Hestbech*
