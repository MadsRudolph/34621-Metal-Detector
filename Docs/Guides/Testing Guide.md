# Testguide - VLF Metaldetektor

> [!abstract] Dokumentformål
> Samlet testguide for verifikation af metaldetektor firmware og hardware.
> Indeholder tre testmetoder: Analog Discovery 3, RC Loopback, og MATLAB DFT verifikation.

---

## Indholdsfortegnelse

1. [[#1. Oversigt]]
2. [[#2. Test med Analog Discovery 3]]
3. [[#3. RC Loopback Test]]
4. [[#4. DFT Verifikation med MATLAB]]
5. [[#5. Fejlfinding]]

---

## 1. Oversigt

### 1.1 Testmetoder

| Test | Formål | Udstyr Krav |
|------|--------|-------------|
| **AD3 Test** | Verificer TX/RX signaler med eksternt udstyr | Analog Discovery 3 |
| **RC Loopback** | Verificer fasemåling med passive komponenter | 1kΩ modstand, 100nF kondensator |
| **MATLAB DFT** | Verificer DFT algoritme mod FFT | MATLAB, USB kabel |

### 1.2 Anbefalet Testrækkefølge

```text
RC Loopback ──▶ AD3 Test ──▶ MATLAB DFT ──▶ Hardware Integration
```

1. **RC Loopback først** - kræver kun passive komponenter
2. **AD3 Test** - mere detaljeret signalanalyse
3. **MATLAB DFT** - matematisk verifikation

---

## 2. Test med Analog Discovery 3

### 2.1 Formål

Test metaldetektorens firmware uden spoler ved at bruge AD3 til at:
- **Måle** TX-udgangssignalet på Pin D9
- **Generere** et testsignal ind på Pin A0 (simulerer RX-spole)
- **Verificere** at DFT-behandlingen fungerer korrekt

### 2.2 Udstyr

| Udstyr | Noter |
|--------|-------|
| Arduino Nano | Programmeret med firmware |
| Analog Discovery 3 | Med WaveForms software |
| Breadboard | Med Nano og OLED forbundet |
| Jumperwires | Til AD3 forbindelser |

### 2.3 AD3 Pin Reference

```text
Analog Discovery 3 Connector:
┌─────────────────────────────────────┐
│  1+  1-  2+  2-  W1  W2  T1  T2 ... │
│  ●   ●   ●   ●   ●   ●   ●   ●      │
│  Scope Ch1  Scope Ch2  Wavegen      │
└─────────────────────────────────────┘

1+/1- = Oscilloskop Kanal 1
W1    = Bølgegenerator 1 udgang
GND   = Stel
```

### 2.4 Forbindelser

| AD3 Pin | Arduino Pin | Formål |
|---------|-------------|--------|
| **1+** (orange) | D9 | Mål TX-udgang |
| **1-** (orange/hvid) | GND | Scope reference |
| **W1** (gul) | A0 | Injicer testsignal |
| **GND** (sort) | GND | Fælles stel |

```text
    ANALOG DISCOVERY 3                    ARDUINO NANO
    ┌──────────────────┐                  ┌──────────────┐
    │  1+ (orange) ────┼──────────────────┼── D9 (TX)    │
    │  1- (org/hvid) ──┼────┐             │              │
    │  W1 (gul) ───────┼────┼─────────────┼── A0 (RX)    │
    │  GND (sort) ─────┼────┴─────────────┼── GND        │
    └──────────────────┘                  └──────────────┘
```

### 2.5 Test 1: Verificer TX-Udgang

**Mål:** Bekræft at Pin D9 udsender en 2 kHz firkantbølge.

**WaveForms Scope Indstillinger:**

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, DC kobling |
| Område | 0-5V |
| Tid/Div | 200 µs |
| Trigger | Ch1, Stigende, 2.5V |

**Forventet Resultat:**

| Måling | Forventet | Tolerance |
|--------|-----------|-----------|
| Frekvens | 2.000 kHz | ±1% |
| Periode | 500 µs | ±5 µs |
| Duty Cycle | 50% | ±2% |
| Amplitude | 5V | - |

### 2.6 Test 2: Injicer Testsignal

**Mål:** Send en 2 kHz sinusbølge ind på A0 og verificer DFT.

**WaveForms Wavegen Indstillinger:**

| Parameter | Værdi |
|-----------|-------|
| Type | Sinus |
| Frekvens | 2000 Hz |
| Amplitude | 2 V (Vpp) |
| Offset | 2.5 V |

**Forventet Display Output:**

| Værdi | Forventet |
|-------|-----------|
| Re/Im | Ikke-nul |
| Mag | ~60-120 for 2Vpp |
| Fase | Roterer (usynkroniseret) |

> [!warning] Fasedrift er Normal
> AD3 og Arduino har uafhængige clocks. Fasen vil rotere langsomt - dette bekræfter at fasedetektion virker!

---

## 3. RC Loopback Test

### 3.1 Formål

Verificer fasemåling ved at bruge et RC filter til at skabe et kendt faseskift. Kræver kun passive komponenter.

### 3.2 Teori

**Basisfase:** TX firkantbølgen giver en basisfase på **-45°** (uden filter).

**RC Filter Faseskift:** Et RC lavpasfilter tilføjer ekstra faseskift:
$$\text{Faseskift} \approx -\arctan(2\pi f R C)$$

**Total målt fase:** Basisfase + RC skift

### 3.3 Komponenter

| Komponent | Værdi | Noter |
|-----------|-------|-------|
| Modstand | 1 kΩ | 1/4W |
| Kondensator | 100 nF | Keramisk/film |
| Jumperwires | 2 stk | |

### 3.4 Kredsløb

```text
                    R (1kΩ)
    D9 (TX) ───────/\/\/\/───────┬─────── A0 (RX)
                                 │
                              ───┴───
                              ───┬─── C (100nF)
                                 │
    GND ─────────────────────────┴─────── GND
```

**Breadboard:**
1. D9 → 1kΩ modstand
2. Modstand → A0 + Kondensator
3. Kondensator → GND

### 3.5 Forventede Resultater

| Konfiguration | Forventet Fase | Tolerance |
|---------------|----------------|-----------|
| Kun modstand (ingen C) | -45° | ±5° |
| 1kΩ + 100nF | -145° | ±15° |

### 3.6 Alternative RC Værdier

| Modstand | Kondensator | Målt Fase |
|----------|-------------|-----------|
| Enhver | Ingen | -45° (basis) |
| 560 Ω | 100 nF | ~ -100° |
| 1 kΩ | 100 nF | ~ -145° |
| 2.2 kΩ | 100 nF | ~ -160° |

> [!tip] Verifikation
> Øgning af R eller C bør gøre fasen mere negativ.
> Fjernelse af kondensator bør returnere til -45°.

### 3.7 Godkendelseskriterier

- [ ] Uden kondensator: Fase ≈ -45° (±5°)
- [ ] Med 1kΩ + 100nF: Fase ≈ -145° (±15°)
- [ ] Faseaflæsning er stabil (hopper ikke)
- [ ] Magnitude er ikke-nul og stabil

---

## 4. DFT Verifikation med MATLAB

### 4.1 Formål

Verificer at Arduinoens **optimerede 4-sample DFT** matcher **MATLAB FFT**.

| Implementation | Metode |
|----------------|--------|
| **Arduino** | Optimeret 4-sample akkumulering (kun bin k=16) |
| **MATLAB** | `X = fftshift(fft(x))/N` (DSP kursus konvention) |

### 4.2 Krav

**Hardware:**
- Arduino Nano med firmware (`CAPTURE_SAMPLES = 1`)
- USB-forbindelse

**Software:**
- MATLAB (R2019b eller nyere)
- Firmware uploadet: `pio run -t upload`

### 4.3 Brug

```matlab
% I MATLAB, naviger til Docs/Matlab/ og kør:
verify_dft
```

1. Vælg COM-port fra listen
2. MATLAB fanger automatisk 64 samples
3. Beregner FFT og sammenligner med Arduino
4. Viser PASS/FAIL og grafer

### 4.4 Forventet Output

```text
========================================
   DFT Verification - VLF Metal Detector
========================================

         Arduino    MATLAB     Error
Re:       -11539    -11539        0
Im:        -7982     -7982        0
Mag:         221       221        0
Phase:      -145      -145        0°

*** PASS - Arduino optimization matches standard FFT! ***
```

### 4.5 Grafisk Output

Scriptet genererer fire grafer i `Docs/Images/`. Disse bruges til at verificere at Arduino DFT matcher MATLAB FFT.

#### 4.5.1 Tidsdomæne Samples
Viser de 64 ADC samples (DC fjernet):

![[../Images/dft_samples.png]]

#### 4.5.2 FFT Spektrum
Centreret magnitude spektrum ($-F_s/2$ til $+F_s/2$) med ±2kHz fremhævet:

![[../Images/dft_spectrum.png]]

> [!info] Konjugeret Symmetri
> For reelle signaler gælder $|X(-f)| = |X(f)|$.
> En cosinus ved 2kHz giver peaks ved **både** +2kHz og -2kHz:
> $$\cos(2\pi f t) = \frac{e^{j2\pi f t} + e^{-j2\pi f t}}{2}$$
> Arduino behøver kun beregne +2kHz da amplituden er identisk.

#### 4.5.3 Phasor Diagram
Sammenligning af Arduino (optimeret) og MATLAB FFT phasors:

![[../Images/dft_phasor.png]]

#### 4.5.4 Verifikations-Sammenfatning
Oversigt over metode og resultater:

![[../Images/dft_comparison.png]]

> [!note] Opdater graferne
> Kør `verify_dft` i MATLAB for at generere nye grafer med aktuelle data.

### 4.6 Matematisk Baggrund

#### Arduino: Optimeret Single-Bin DFT

Arduino udnytter at vi sampler ved **4× signalfrekvensen**:
- $f_s = 8000$ Hz, $f_{signal} = 2000$ Hz
- Bin 16 svarer til 2kHz: $k = f_{signal} \cdot N / f_s = 2000 \cdot 64 / 8000 = 16$

Ved bin 16 bliver DFT koefficienterne trivielle:

| Sample n mod 4 | cos(2πn/4) | -sin(2πn/4) | Operation |
|----------------|------------|-------------|-----------|
| 0 | +1 | 0 | Re += x[n] |
| 1 | 0 | -1 | Im -= x[n] |
| 2 | -1 | 0 | Re -= x[n] |
| 3 | 0 | +1 | Im += x[n] |

#### MATLAB: DSP Kursus Konvention (62743)

Vi bruger den normaliserede, centrerede FFT:
$$X[k] = \frac{1}{N} \sum_{n=0}^{N-1} x[n] \cdot e^{-j2\pi kn/N}$$

```matlab
% DSP kursus konvention: normaliseret og centreret
X = fftshift(fft(samples)) / N;
f_axis = (-N/2:N/2-1) * (Fs/N);    % Frekvensakse [Hz]

% Find 2kHz bin via frekvensakse
[~, idx] = min(abs(f_axis - 2000));
X_2kHz = X(idx);

% Skaler tilbage for Arduino sammenligning
Re = real(X_2kHz) * N;
Im = imag(X_2kHz) * N;
```

### 4.7 Tolerancer

| Parameter | Tolerance | Beskrivelse |
|-----------|-----------|-------------|
| Magnitude | ±5 | Efter division med N |
| Fase | ±3° | Grader |

> [!tip] Forventede afvigelser
> Små afvigelser skyldes floating-point vs integer afrunding.
> Re/Im bør matche præcist da begge metoder bruger samme samples.

### 4.8 Firmware Konfiguration

I `Code/src/main.c`:

```c
#define CAPTURE_SAMPLES 1    // Aktiver capture mode
#define UART_BAUD 115200     // Serial hastighed
```

### 4.9 Serial Protokol

| Kommando | Respons | Beskrivelse |
|----------|---------|-------------|
| (connect) | `READY` | Arduino klar efter reset |
| `C` | `ACK` + data | Trigger capture af 64 samples |

### 4.10 Filer

| Fil | Placering | Beskrivelse |
|-----|-----------|-------------|
| `verify_dft.m` | `Docs/Matlab/` | Verifikationsscript (DSP konvention) |
| `dft_samples.png` | `Docs/Images/` | Tidsdomæne plot |
| `dft_spectrum.png` | `Docs/Images/` | Centreret FFT spektrum (±4kHz) |
| `dft_phasor.png` | `Docs/Images/` | Phasor diagram |
| `dft_comparison.png` | `Docs/Images/` | Resultat-sammenfatning |

---

## 5. Fejlfinding

### 5.1 Generelle Problemer

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Intet TX signal | Firmware kører ikke | Tjek upload, verificer `sei()` kaldes |
| TX forkert frekvens | Timer0 config forkert | Verificer OCR0A = 249, prescaler = 8 |
| Mag altid 0 | Intet signal på A0 | Tjek forbindelser |
| Display opdaterer ikke | DFT_done ikke sat | Tjek ADC ISR kører |

### 5.2 AD3-Specifikke Problemer

| Problem | Løsning |
|---------|---------|
| Fase "over det hele" | **Normalt!** Bekræfter fase virker (uafhængige clocks) |
| Intet wavegen signal | Tjek W1 forbindelse, verificer Wavegen kører |

### 5.3 RC Loopback Problemer

| Problem | Løsning |
|---------|---------|
| Fase nær -45° med kondensator | Kondensator ikke forbundet korrekt |
| Fase hopper | Løse forbindelser, brug kortere ledninger |
| Magnitude meget lav | Verificer modstand er 1kΩ, ikke 1MΩ |

### 5.4 MATLAB Problemer

| Problem | Løsning |
|---------|---------|
| "No serial ports available" | Tjek USB-forbindelse |
| Timeout ved READY | Vent på Arduino reset |
| Parsed 0 samples | Prøv igen, tjek `CAPTURE_SAMPLES = 1` |

---

## 6. Relaterede Dokumenter

- [[Assembly_Guide|Samlevejledning]] - Hardware opsætning
- [[Code_Review|Firmware Gennemgang]] - Kodestruktur
- [[../Theory/DFT Algorithm|DFT Algoritme]] - Matematisk baggrund

---

*Testguide til DTU 34621 Metaldetektor Projekt*
