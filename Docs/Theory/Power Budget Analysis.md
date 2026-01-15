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
| **TX Spole (RMS)** | **360** | H-bro design |
| **Total Elektronik** | **~40 mA** | Høj |
| **Total System** | **~400 mA** | Høj |

### 5.2 TX Strøm Beregning

H-bro forstærkeren leverer 18 Vpp til RLC tank med total modstand 35Ω:

$$I_{peak} = \frac{V_{pp}}{R_{total}} = \frac{18V}{35\Omega} = 514\ \text{mA}$$

$$I_{RMS} = \frac{I_{peak}}{\sqrt{2}} = \frac{514}{\sqrt{2}} = 363\ \text{mA} \approx 360\ \text{mA}$$

> [!warning] Høj TX Strøm
> TX systemet trækker **360 mA RMS**, hvilket er væsentligt højere end tidligere estimater.
> Dette påvirker køretiden betydeligt.

### 5.3 Total Systemeffekt

$$P_{total} = V_{bat} \times I_{total}$$

| Batteri Tilstand | $V_{bat}$ (V) | $I_{total}$ (mA) | $P_{total}$ (W) |
|------------------|---------------|------------------|-----------------|
| Frisk | 9.0 | 400 | **3.6** |
| 75% kapacitet | 8.0 | 400 | **3.2** |
| 50% kapacitet | 7.5 | 400 | **3.0** |
| Slut på levetid | 6.5 | 400 | **2.6** |

> [!danger] Strømbudget Overskridelse
> Total systemstrøm på **400 mA** overstiger 9V batteriets praktiske grænse.
> Se afsnit 7 for løsningsmuligheder.

---

## 6. TX Spole og Driver

> [!info] Detaljeret Dokumentation
> - **Spole specifikationer:** Se [[Coil Design|Spole Design]]
> - **Driver kredsløb:** Se [[TX Driver Design|TX Driver Design]]

### 6.1 Nøgletal for Strømbudget

| Parameter | Værdi | Kilde |
|-----------|-------|-------|
| TX induktans | 6.33 mH | Spole Design |
| Spole DC modstand | ~3 Ω | 0.56mm tråd |
| Serie modstand | 32 Ω (5W) | Forstærker matching |
| Total modstand | 35 Ω | R_coil + R_serie |
| H-bro spænding | 18 Vpp | 2 × 9V batteri |
| **TX strøm (RMS)** | **360 mA** | Beregnet |
| Effekt i serie R | ~4.1 W | $I^2 \times R$ |
| Driver effektivitet | >98% | QSPICE simulation |

---

## 7. Komplet Strømbudget Oversigt

### 7.1 Opdateret Strømbudget (H-bro Design)

```
┌──────────────────────────────────────────────────────────────────────┐
│               STRØMBUDGET - H-BRO FORSTÆRKER                        │
│                   (RLC Tank: 6.33mH + 32Ω + 1µF)                    │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  BATTERI: Duracell MN1604 (9V alkalisk)                             │
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
│  TX SPOLE SYSTEM (H-bro, 18 Vpp, 35Ω total)           360 mA       │
│  ├─ TX Spole (L=6.33mH, R_dc=3Ω)                                   │
│  ├─ Serie Modstand (32Ω, 5W)                          ~4.1 W tab   │
│  └─ Driver tab (MOSFET)                               <0.5 mW       │
│                                                                      │
├──────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  TOTAL SYSTEM STRØM                                   400 mA        │
│  FORVENTET KØRETID (enkelt 9V)                       ~30-40 min    │
│                                                                      │
│  TX STRØM (RMS)                                       360 mA        │
│                                                                      │
│  STATUS: ⚠️ OVERSTIGER 9V BATTERI KAPACITET                        │
│                                                                      │
└──────────────────────────────────────────────────────────────────────┘
```

> [!danger] Køretidsproblem
> Med 400 mA total strøm kan et enkelt 9V batteri kun levere **~30-40 minutter** køretid.
> Dette opfylder IKKE kravet om 100 minutter.

### 7.2 Køretidsestimater

Fra afladningsdata (sektion 1.3) kan vi ekstrapolere:

| Total Strøm | Estimeret Køretid | Status |
|-------------|-------------------|--------|
| 200 mA | ~75 min | Under krav |
| 300 mA | ~45 min | Under krav |
| **400 mA** | **~30 min** | **Langt under krav** |

### 7.3 Løsningsmuligheder

#### Option A: Reduceret TX Duty Cycle

| Duty Cycle | TX Strøm (avg) | Total Strøm | Køretid |
|------------|----------------|-------------|---------|
| 100% | 360 mA | 400 mA | ~30 min |
| 50% | 180 mA | 220 mA | ~65 min |
| 25% | 90 mA | 130 mA | ~95 min |
| **20%** | **72 mA** | **112 mA** | **~100 min** ✓ |

> [!tip] Pulserende TX
> Ved at køre TX med ~20% duty cycle kan køretidskravet opfyldes.
> Dette reducerer dog detektionsfølsomheden.

#### Option B: Ekstern Strømforsyning

| Strømkilde | Kapacitet | Køretid @ 400mA |
|------------|-----------|-----------------|
| 9V alkalisk | ~250 mAh @ 400mA | ~35 min |
| 2× 9V parallel | ~500 mAh | ~70 min |
| **6× AA (9V)** | **~2000 mAh** | **~5 timer** ✓ |
| USB powerbank | 5000+ mAh | Mange timer |

#### Option C: Højere Modstand (Lavere Strøm)

| Total R | TX Strøm | Total Strøm | Køretid |
|---------|----------|-------------|---------|
| 35 Ω | 360 mA | 400 mA | ~30 min |
| 70 Ω | 180 mA | 220 mA | ~65 min |
| **150 Ω** | **85 mA** | **125 mA** | **~100 min** ✓ |

> [!note] Trade-off
> Højere modstand = lavere strøm = længere køretid, men også svagere magnetfelt.

---

## 8. Strømstyringsstrategi

### 8.1 Design for 7.5V Forsyningsspænding

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

### 8.2 Driftstilstande

> [!warning] Bemærk
> Med nuværende design (360 mA TX) kan 100 min køretid kun opnås med reduceret duty cycle eller alternativ strømkilde.

| Tilstand | TX Duty | TX Strøm (avg) | Total Strøm | Køretid |
|----------|---------|----------------|-------------|---------|
| **Fuld ydelse** | 100% | 360 mA | 400 mA | ~30 min |
| Pulserende | 50% | 180 mA | 220 mA | ~65 min |
| Batterisparer | 25% | 90 mA | 130 mA | ~95 min |
| **Kravopfyldelse** | **20%** | **72 mA** | **112 mA** | **~100 min** |

> [!tip] Anbefaling
> For at opfylde 100 min køretidskravet:
> - **Option A:** Brug ~20% TX duty cycle (reduceret følsomhed)
> - **Option B:** Brug ekstern strømkilde (6× AA batterier)
> - **Option C:** Øg serie modstand til ~150Ω (reduceret magnetfelt)

---

## 9. Verifikationstest Plan

### 9.1 Komponenttest

| Test | Opsætning | Bestået Kriterier |
|------|-----------|-------------------|
| Arduino Nano + OLED | AD3 @ 9V via shunt | < 45 mA |
| TX driver alene | Oscilloskop + DMM | 350-370 mA RMS |
| Fuld elektronik (uden TX) | Shunt + batteri | < 45 mA |
| Fuldt system (100% duty) | Shunt + batteri | 390-410 mA |

### 9.2 Spole Verifikation

| Test | Metode | Bestået Kriterier |
|------|--------|-------------------|
| TX induktans | LCR meter @ 1kHz | 6-7 mH |
| TX DC modstand | Multimeter | 2-4 Ω |
| Serie modstand | Multimeter | 32 Ω ±5% |
| Total modstand | Multimeter | 34-36 Ω |

### 9.3 Køretids Test Protokol

> [!note] Test Scenarie
> Følgende test antager **20% TX duty cycle** for at opfylde køretidskravet.

| Tid (min) | Mål I (mA) | $V_{bat}$ (V) | Status |
|-----------|------------|---------------|--------|
| 0 | ~112 | >9.0 | Start |
| 25 | ~112 | >8.0 | |
| 50 | ~112 | >7.5 | |
| 75 | ~112 | >7.0 | |
| **100** | ~112 | **>6.0** | **BESTÅET?** |

> [!warning] Alternativ Test
> Ved 100% TX duty cycle (400 mA) vil batteriet være opbrugt efter ~30 min.

---

## 10. Referencer

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

#strøm #beregninger #spole-design #driver #målinger #krav
