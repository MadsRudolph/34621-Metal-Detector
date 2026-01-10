# Strømbudget Analyse

> [!abstract] Kernekrav
> Metaldetektoren skal køre i **100 minutter** på et 9V batteri (6LR61) med resterende spænding **>6V**.
>
> Se: [[kravspecifikation.pdf|Kravspecifikation]] krav 4 & 5

---

## 1. Batteri Karakteristik

### 1.1 Duracell MN1604 Specifikationer

> **Datablad:** [[Literature/duracell_9volt.pdf|Duracell 9V Datablad]]

| Parameter | Værdi | Betingelser |
|-----------|-------|-------------|
| Nominel spænding | 9.0 V | Åben kreds |
| Frisk spænding | 9.4 - 9.6 V | Ingen belastning |
| Driftsområde | 9.6 V - 4.8 V | Under belastning |
| Indre impedans | 1.7 Ω | @ 1 kHz, frisk |
| Indre impedans | 3-5 Ω | @ 1 kHz, afladet |
| Nominel kapacitet | 580 mAh | Til 4.8V @ 25mA |
| Vægt | 45.6 g | |
| Kemi | Alkalisk (Zn-MnO₂) | |

### 1.2 Kapacitet vs. Afladningsrate

Alkaliske batterier udviser reduceret effektiv kapacitet ved højere afladningsrater på grund af indre modstand og elektrokemiske begrænsninger.

**Peukerts ligning (modificeret for alkalisk):**

$$C_{eff} = C_{nom} \times \left(\frac{I_{nom}}{I_{actual}}\right)^{n-1}$$

Hvor:
- $C_{eff}$ = effektiv kapacitet (mAh)
- $C_{nom}$ = nominel kapacitet ved standardrate (~500 mAh)
- $I_{nom}$ = nominel afladningsrate (~25 mA for 9V alkalisk)
- $I_{actual}$ = faktisk afladningsstrøm
- $n$ = Peukert eksponent (~1.1 for alkalisk)

### 1.3 Afladningskurve Analyse

Fra datablad kurver ved 21°C:

| Strøm (mA) | Køretid til 6V | Effektiv Kapacitet | Effektivitet |
|------------|----------------|-------------------|--------------|
| 10 | 2400 min (40t) | 400 mAh | 80% |
| 25 | 1080 min (18t) | 450 mAh | 90% |
| 50 | 600 min (10t) | 500 mAh | 100% |
| 100 | 210 min (3.5t) | 350 mAh | 70% |
| 150 | 120 min (2t) | 300 mAh | 60% |
| 200 | 75 min (1.25t) | 250 mAh | 50% |

---

## 2. Køretidskrav Udledning

### 2.1 Kravspecifikation

Fra kravspecifikation:
- Minimum køretid: $t_{min} = 100$ minutter
- Slutspænding: $V_{end} > 6.0V$

### 2.2 Maksimal Strømberegning

**Metode 1: Lineær interpolation fra afladningsdata**

Fra datablad:
- Ved $I_1 = 100$ mA → $t_1 = 210$ min
- Ved $I_2 = 150$ mA → $t_2 = 120$ min

For $t_{target} = 100$ min:

$$I_{max} = 100 + \frac{(210 - 100)}{(210 - 120)} \times (150 - 100) = 161\ \text{mA}$$

**Metode 2: Konservativt estimat med usikkerheder**

Faktorer der reducerer faktisk kapacitet:
- Temperaturvariation: -10%
- Batteri aldring: -5%
- Produktionsvariation: -5%

Kombineret nedvurderingsfaktor: $0.9 \times 0.95 \times 0.95 = 0.81$

$$I_{konservativ} = 161 \times 0.81 = 130\ \text{mA}$$

> [!danger] Design Maksimum
> $I_{max} = 120$ mA (afrundet ned for margin)

### 2.3 Designmål med Sikkerhedsmargin

$$I_{target} = \frac{I_{max}}{1.2} = \frac{120}{1.2} = 100\ \text{mA}$$

> [!success] Designmål
> $I_{system} \leq 100$ mA (konservativ) eller $\leq 120$ mA (maksimal)

---

## 3. Arduino Nano Strømanalyse

### 3.1 Board Blokdiagram

```
                    ┌─────────────────────────────────────────┐
                    │           Arduino Nano                   │
                    │          (ATmega328P)                    │
                    │                                         │
    9V Batteri ────►│──► AMS1117 ──► 5V Rail ──┬──► ATmega328P│
         │          │    (LDO)        │        │              │
         │          │                 │        ├──► CH340/FTDI│
         │          │                 │        │   (USB chip) │
         │          │                 │        ├──► Power LED │
         │          │                 │        └──► OLED      │
         │          │                 │                       │
         └──────────│─────────────────┴─► 3.3V Regulator     │
                    │                                         │
                    └─────────────────────────────────────────┘
```

### 3.2 Komponent-for-Komponent Analyse

#### 3.2.1 ATmega328P Mikrocontroller

**Fra ATmega328P Datablad, Sektion 29.2:**

| Tilstand | Betingelser | Strøm |
|----------|-------------|-------|
| Aktiv | 16 MHz, 5V | 12 mA (typ), 20 mA (max) |
| Idle | 16 MHz, 5V | 3.6 mA |
| Power-save | 32 kHz RTC, 5V | 0.9 mA |
| Power-down | WDT aktiveret, 5V | 15 µA |
| Power-down | WDT deaktiveret, 5V | 0.1 µA |

**Periferi strømtillæg:**

| Periferi | Ekstra Strøm | Noter |
|----------|--------------|-------|
| ADC | 0.3 mA | Under konvertering |
| Timer0 | 0.1 mA | TX generering |
| TWI (I2C) | 0.1 mA | OLED display |
| Brown-out Detector | 0.02 mA | Altid aktiv som standard |

**Estimeret ATmega328P total (vores applikation):**

$$I_{MCU} = I_{aktiv} + I_{ADC} + I_{Timer0} + I_{TWI} = 12 + 0.3 + 0.1 + 0.1 = 12.5\ \text{mA (typisk)}$$

Med datasheet maksimum for sikkerhed: **$I_{MCU} = 15$ mA**

#### 3.2.2 USB Interface Chip (CH340 eller FTDI)

De fleste Arduino Nano kloner bruger CH340G til USB-til-Serial konvertering.

**Fra CH340G datablad:**

| Tilstand | Strøm |
|----------|-------|
| Aktiv (USB forbundet) | 15 mA |
| Standby (USB afbrudt) | 2-3 mA |

> [!note] CH340 vs ATmega16U2
> Original Arduino Nano bruger FTDI chip. Mange kloner bruger CH340G som trækker mindre strøm end ATmega16U2 (12 mA) på Arduino Mega.

**$I_{USB\_chip} = 3$ mA** (USB afbrudt, kun serial)

#### 3.2.3 Power LED

Grøn LED med strømbegrænsende modstand.

**Kredsløb:** 5V → LED ($V_f \approx 2.1V$) → R → GND

For typisk 1kΩ modstand:
$$I_{LED} = \frac{5V - 2.1V}{1000\Omega} = 2.9\ \text{mA}$$

**$I_{LED} = 3$ mA**

#### 3.2.4 Spændingsregulator Analyse

AMS1117 (eller lignende) er en lineær (LDO) regulator.

**Lineær regulator grundlæggende:**

$$I_{ind} = I_{ud} + I_q$$

**AMS1117 specifikationer:**
- Quiescent strøm: $I_q = 5$ mA (typisk)
- Dropout spænding: 1.1V @ 800mA

**Effekttab i regulator:**

Ved $V_{ind} = 9V$, $V_{ud} = 5V$, $I_{ud} = 40mA$:

$$P_{tab} = (V_{ind} - V_{ud}) \times I_{ud} = 4V \times 40mA = 160\ \text{mW}$$

**Regulator overhead: $I_q = 5$ mA**

#### 3.2.5 SSD1306 OLED Display

**Fra SSD1306 datablad:**

| Parameter | Værdi |
|-----------|-------|
| Forsyningsspænding | 3.3V - 5V |
| Standby strøm | 10 µA |
| Driftsstrøm (50% pixels) | 8-10 mA |
| Driftsstrøm (alle pixels TIL) | 20 mA |

For typisk tekst display (~30% pixels tændt):

**$I_{OLED} = 12$ mA**

### 3.3 Total Board Strømoversigt

| Komponent | Strøm (mA) | Noter |
|-----------|------------|-------|
| ATmega328P (aktiv) | 15 | Konservativt estimat |
| CH340G (USB chip) | 3 | Standby tilstand |
| Power LED | 3 | Grøn LED |
| AMS1117 quiescent | 5 | LDO overhead |
| SSD1306 OLED | 12 | ~30% pixels |
| PCB lækage | 2 | Kondensatorer, spor |
| **Total (5V rail)** | **40 mA** | |

### 3.4 Sammenligning: Arduino Nano vs Arduino Mega

| Parameter | Arduino Nano | Arduino Mega 2560 | Besparelse |
|-----------|--------------|-------------------|------------|
| MCU | ATmega328P | ATmega2560 | |
| MCU strøm (aktiv) | 12 mA | 14 mA | 2 mA |
| MCU strøm (max) | 15 mA | 25 mA | 10 mA |
| USB chip | CH340G (3 mA) | ATmega16U2 (12 mA) | **9 mA** |
| Power LED | 3 mA | 8 mA | 5 mA |
| 3.3V system | Minimal | 2 mA | 2 mA |
| **Total elektronik** | **~40 mA** | **~80 mA** | **~40 mA** |

> [!success] Arduino Nano Fordel
> Arduino Nano bruger cirka **halvt så meget strøm** som Arduino Mega, hvilket giver mere strøm til TX spolen!

---

## 4. DSP Overhead — Detaljeret Analyse

### 4.1 Timer0 (TX Signalgenerering & ADC Trigger)

**Konfiguration:**
- Mode: CTC (Clear Timer on Compare)
- Interrupt rate: 8 kHz
- TX toggle hver 2. interrupt = 2 kHz firkantbølge
- ADC auto-trigger fra Timer0 Compare Match A

**Strømanalyse:**

Timer0 tilføjer cirka:
$$\Delta I_{Timer0} \approx 0.1\ \text{mA}$$

**Total Timer0 overhead: ~0.1 mA** (ubetydelig)

### 4.2 ADC Konvertering

**Konfiguration:**
- Opløsning: 10-bit
- Clock: 250 kHz (16 MHz / 64)
- Konverteringstid: 13 ADC cykler = 52 µs
- Sample rate: 8 kHz (auto-triggered)

52 µs konverteringstid vs 125 µs sample periode betyder ADC er aktiv ~42% af tiden.

**ADC strøm:**

Fra datablad: ADC tilføjer 0.3 mA når aktiv.

Ved 42% duty cycle:
$$I_{ADC} = 0.3\ \text{mA} \times 0.42 = 0.13\ \text{mA}$$

**Total ADC overhead: ~0.15 mA**

### 4.3 DFT Beregning

**Algoritme:** Single-bin DFT med 4× oversampling optimering

Se [[DFT Algorithm|DFT Algoritme]] for detaljer.

**Strømpåvirkning:**

CPU'en kører allerede. DFT beregning øger ikke strømmen — den bruger bare CPU cykler.

**DFT overhead: 0 mA** (ingen ekstra strøm, kun CPU tid)

### 4.4 Total DSP Overhead

| Komponent | Strøm (mA) |
|-----------|------------|
| Timer0 periferi | 0.1 |
| ADC (42% aktiv) | 0.15 |
| DFT beregning | 0 |
| **Total DSP overhead** | **~0.3 mA** |

---

## 5. Revideret Strømbudget

### 5.1 Endelig Strømopdeling

| Komponent | Strøm (mA) | Konfidens |
|-----------|------------|-----------|
| **Elektronik baseline** | **40** | Beregnet |
| **DSP tillæg** | **0.3** | Beregnet |
| **Total Elektronik** | **~40 mA** | Høj |

### 5.2 Tilgængelig Strøm til TX Spole

$$I_{TX,tilgængelig} = I_{max} - I_{elektronik}$$

**Konservativ tilstand (100 mA budget):**
$$I_{TX} = 100\ \text{mA} - 40\ \text{mA} = 60\ \text{mA}$$

**Maksimal tilstand (120 mA budget):**
$$I_{TX} = 120\ \text{mA} - 40\ \text{mA} = 80\ \text{mA}$$

> [!tip] Designmål for TX Spole
> **60-80 mA** tilgængelig for TX spole driver — **dobbelt så meget som Arduino Mega!**

### 5.3 Tilgængelig Effekt til TX Spole

$$P_{TX} = V_{forsyning} \times I_{TX}$$

| Batteri Tilstand | $V_{forsyning}$ (V) | $I_{TX}$ (mA) | $P_{TX}$ (mW) |
|------------------|---------------------|---------------|---------------|
| Frisk | 9.0 | 80 | **720** |
| 75% kapacitet | 8.0 | 80 | **640** |
| 50% kapacitet | 7.5 | 80 | **600** |
| 25% kapacitet | 7.0 | 80 | **560** |
| Slut på levetid | 6.5 | 80 | **520** |
| Cutoff | 6.0 | 80 | **480** |

> [!info] TX Spole Strømbudget
> **480-720 mW** tilgængelig (varierer med batterispænding)
> Dette er **dobbelt** så meget som med Arduino Mega!

---

## 6. TX Spole Design Beregninger

> **Detaljerede spole designs:** Se [[Search Coil Design]]

### 6.1 Trådspecifikation

| Parameter | Værdi | Noter |
|-----------|-------|-------|
| **Tråddiameter** | **0.52 mm** | AWG 24 ækvivalent |
| Trådtype | Emaljeret kobber | Magnetledning |
| Modstand per meter | **0.079 Ω/m** | |
| Strømkapacitet | 1.2 A | Langt over 80 mA |

### 6.2 Påkrævet Impedans Beregning

For målstrøm $I_{spole}$ ved forsyningsspænding $V$:

$$|Z_{spole}| = \frac{V_{forsyning}}{I_{spole}}$$

| $V_{forsyning}$ | $I_{spole}$ | $|Z|$ |
|-----------------|-------------|-------|
| 9.0 V | 80 mA | 112.5 Ω |
| 7.5 V | 80 mA | 93.75 Ω |
| 6.0 V | 80 mA | 75 Ω |

**Designmål: $|Z| \approx 95\ \Omega$** (for mid-life batterispænding @ 7.5V)

### 6.3 Induktans Beregning

Ved 2 kHz, antaget $R_{dc} \ll X_L$:

$$|Z| \approx X_L = 2\pi f L$$

$$L = \frac{|Z|}{2\pi f} = \frac{95}{2\pi \times 2000} = 7.5\ \text{mH}$$

**Mål TX spole induktans: L ≈ 7-8 mH** (for 80 mA @ 7.5V)

> [!note] Sammenligning med Arduino Mega Design
> Med Arduino Mega var målimpedansen ~190Ω for 40 mA.
> Med Arduino Nano kan vi bruge ~95Ω for 80 mA — **dobbelt strøm = dobbelt magnetfelt!**

### 6.4 Koncentrisk Spole Design (Opdateret til Nano)

| Parameter | Mega Design | Nano Design | Ændring |
|-----------|-------------|-------------|---------|
| TX diameter | 150 mm | 150 mm | Samme |
| TX vindinger | 130 | ~90 | Færre |
| TX induktans | 15 mH | 7.5 mH | Halveret |
| TX impedans @ 2kHz | 189 Ω | 95 Ω | Halveret |
| TX strøm @ 7.5V | 40 mA | **80 mA** | **Dobbelt** |
| Magnetisk felt | Baseline | **~200%** | **Dobbelt** |

### 6.5 Detektionsdybde Forbedring

Detektionsdybde skalerer med kubikroden af magnetisk feltstyrke:

$$\text{Dybde} \propto B^{1/3}$$

Med dobbelt TX strøm:
$$\text{Dybdeforbedring} = 2^{1/3} = 1.26 = +26\%$$

> [!success] Arduino Nano Fordel
> Med samme 100 min køretid giver Arduino Nano **~26% dybere detektion** end Arduino Mega!

---

## 7. TX Spole Driver Kredsløb

> **Detaljeret design:** Se [[TX Driver Design]]

### 7.1 Forstærker Topologi

| Aspekt | Klasse D (Switching) | Klasse AB (Lineær) |
|--------|---------------------|-------------------|
| **Effektivitet** | 90-99% | 50-70% |
| **Varmeafledning** | Meget lav | Høj |
| **Effekt til spole** | ~700 mW | ~400 mW |
| **Kompleksitet** | Lav-Medium | Lav |

> [!success] Valgt Topologi
> **Klasse D (Switching)** — Højere effektivitet kritisk for batterilevetid.

### 7.2 MOSFET Half-Bridge Design

```
                              V_bat (6-9V)
                                 │
                            ┌────┴────┐
                            │   Q1    │
                            │ Si2301  │  P-kanal
    Pin 9 (D9) ────[100Ω]───┤G   S   D├───┐
                            └─────────┘   │
                                          │
                                     ┌────┴────┐
                                     │ TX Spole │
                                     │  7.5 mH  │
                                     └────┬────┘
                                          │
                            ┌─────────────┘
                            │
                       ┌────┴────┐
                       │   Q2    │
    Pin 10 ────[100Ω]──┤G  D    S├───► GND
                       │ Si2302  │  N-kanal
                       └─────────┘
```

### 7.3 Driver Effektivitet

| Parameter | Værdi |
|-----------|-------|
| Indgangseffekt (fra batteri) | $7.5V \times 80mA = 600$ mW |
| MOSFET lednings tab | <1 mW |
| Switching tab | <0.1 mW |
| Gate drive tab | <0.5 mW |
| **Total driver tab** | **<2 mW** |
| Effekt til spole | **~598 mW** |
| **Driver effektivitet** | **>99%** |

---

## 8. Komplet Strømbudget Oversigt

### 8.1 Arduino Nano - Maksimal Effekt Tilstand

```
┌──────────────────────────────────────────────────────────────────────┐
│               STRØMBUDGET - ARDUINO NANO                             │
│                   (Koncentrisk Spole, 0.52mm Tråd)                   │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  BATTERI: Duracell MN1604 → ~350 mAh til 6V @ 120mA                 │
│  MÅL KØRETID: 100 min (krav)                                        │
│  TX DUTY CYCLE: 100% (kontinuerlig)                                 │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ELEKTRONIK                                             40 mA       │
│  ├─ Arduino Nano Board                                  35 mA       │
│  │   ├─ ATmega328P (aktiv, alle periferier)   15 mA               │
│  │   ├─ CH340G (USB interface)                 3 mA               │
│  │   ├─ Power LED                              3 mA               │
│  │   ├─ AMS1117 regulator quiescent            5 mA               │
│  │   └─ PCB lækage, afkobling                  9 mA               │
│  ├─ SSD1306 OLED Display                                12 mA       │
│  └─ DSP Overhead                                       0.3 mA       │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TX SPOLE SYSTEM (100% duty cycle)                      80 mA       │
│  ├─ TX Spole (L=7.5mH, Z=95Ω @ 7.5V)                   80 mA       │
│  └─ Driver tab (MOSFET)                               <0.5 mW       │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TOTAL SYSTEM STRØM                                   120 mA        │
│  FORVENTET KØRETID                                   ~100 min       │
│                                                                      │
│  TX STRØM                                              80 mA        │
│  MAGNETISK FELTSTYRKE                           ~200% vs Mega       │
│  DETEKTIONSDYBDE                                +26% vs Mega        │
│                                                                      │
│  STATUS: ✅ OPFYLDER KRAV - Dobbelt felt vs Arduino Mega!           │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

### 8.2 Sammenligning: Nano vs Mega

| Parameter | Arduino Mega | Arduino Nano | Forbedring |
|-----------|--------------|--------------|------------|
| Elektronik strøm | 80 mA | 40 mA | **-50%** |
| Tilgængelig TX strøm | 40 mA | 80 mA | **+100%** |
| TX spole impedans | 189 Ω | 95 Ω | -50% |
| TX spole induktans | 15 mH | 7.5 mH | -50% |
| Magnetisk felt | Baseline | **~200%** | **+100%** |
| Detektionsdybde | Baseline | **+26%** | **+26%** |
| Køretid | 100 min | 100 min | Samme |

![[nano_vs_mega.png]]
*Figur: Arduino Nano vs Mega strømsammenligning (genereret af MATLAB)*

### 8.3 Med LED Fjernet (Bedste Ydeevne)

Fjernelse af power LED frigør 3 mA for mere TX effekt:

| Konfiguration | TX Strøm | Total Strøm | Køretid | Felt |
|---------------|----------|-------------|---------|------|
| Standard | 80 mA | 120 mA | 100 min | 200% |
| LED fjernet | **83 mA** | 120 mA | 100 min | **207%** |

---

## 9. Strømstyringsstrategi

### 9.1 Design for 7.5V Forsyningsspænding

**Problem med at designe for 9V:**
- Frisk batteri: 9.4V under belastning
- Mid-life batteri: 7.5V under belastning
- Slut-på-levetid: 6.0V (cutoff)

**Løsning: Design alle kredsløb for 7.5V**

| Fordel | Forklaring |
|--------|------------|
| **Konsistent ydeevne** | Samme TX strøm fra frisk til næsten opbrugt batteri |
| **Ingen kompensationskode** | Ikke behov for spændingsregistrering eller adaptiv drift |
| **Forudsigelig opførsel** | Detektionsdybde forbliver konstant gennem brug |

### 9.2 Driftstilstande

| Tilstand | TX Duty | TX Strøm | Total Strøm | Køretid | Felt |
|----------|---------|----------|-------------|---------|------|
| **Maksimal** | 100% | 80 mA | 120 mA | 100 min | 200% |
| Konservativ | 80% | 64 mA | 100 mA | 120 min | 160% |
| Batteri Sparer | 50% | 40 mA | 80 mA | 150 min | 100% |

> [!tip] Anbefaling
> Brug **Maksimal tilstand** for bedste detektionsydeevne.
> Arduino Nano's lave strømforbrug betyder vi kan køre TX ved fuld effekt!

### 9.3 Hvorfor Arduino Nano er Bedre

```
┌─────────────────────────────────────────────────────────────────┐
│                STRØMFORDELING SAMMENLIGNING                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ARDUINO MEGA (120 mA budget):                                 │
│  ████████████████████████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ │
│  │◄──── Elektronik 80 mA ────►│◄── TX 40 mA ──►│              │
│                                                                 │
│  ARDUINO NANO (120 mA budget):                                 │
│  ████████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ │
│  │◄─ Elek 40 mA ─►│◄────────── TX 80 mA ──────────►│          │
│                                                                 │
│  Resultat: DOBBELT magnetfelt = +26% detektionsdybde           │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## 10. Verifikationstest Plan

### 10.1 Komponenttest

| Test | Opsætning | Bestået Kriterier |
|------|-----------|-------------------|
| Arduino Nano + OLED | AD3 @ 9V via shunt | < 45 mA |
| TX driver alene | Oscilloskop + DMM | 75-85 mA |
| Fuld elektronik | Shunt + batteri | < 45 mA |
| Fuldt system | Shunt + batteri | 115-125 mA |

### 10.2 Spole Verifikation

| Test | Metode | Bestået Kriterier |
|------|--------|-------------------|
| TX induktans | LCR meter @ 1kHz | 7-8 mH |
| TX modstand | Multimeter | 2-4 Ω |
| TX impedans @ 2kHz | Beregnet | 90-100 Ω |

### 10.3 Køretids Test Protokol

| Tid (min) | Mål I (mA) | $V_{bat}$ (V) | Status |
|-----------|------------|---------------|--------|
| 0 | 120 | >9.0 | Start |
| 25 | 120 | >8.0 | |
| 50 | 120 | >7.5 | |
| 75 | 120 | >7.0 | |
| **100** | 120 | **>6.0** | **BESTÅET?** |

---

## 11. Referencer

1. [[Literature/duracell_9volt.pdf|Duracell 9V Datablad]]
2. [[kravspecifikation.pdf|Kravspecifikation]]
3. [[Search Coil Design]]
4. [[TX Driver Design]]
5. [[DFT Algorithm|DFT Algoritme]]
6. ATmega328P Datablad, Microchip Technology
7. CH340G Datablad, WCH
8. Si2301/Si2302 Datablad, Vishay
9. SSD1306 Datablad, Solomon Systech
10. AMS1117 Datablad, Advanced Monolithic Systems

---

*Dokument opdateret til Arduino Nano (ATmega328P) fra Arduino Mega 2560*

#strøm #beregninger #spole-design #driver #målinger #krav
