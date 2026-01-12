# DFT Verifikation med MATLAB

> [!abstract] Formål
> Verificerer at Arduinoens **optimerede 4-sample DFT** giver samme resultat som **MATLAB FFT**.
> Bruger **DSP kursus konvention** (62743) med `fftshift` og normalisering.

---

## Verifikationsmetode

| Implementation | Metode | Beregner |
|----------------|--------|----------|
| **Arduino** | Optimeret 4-sample akkumulering | Kun bin k=16 (2kHz) |
| **MATLAB** | `X = fftshift(fft(x))/N` | Alle bins, centreret spektrum |

> [!tip] Hvorfor dette virker
> Arduino bruger en optimering hvor cos/sin værdier kun er +1, -1 eller 0.
> Ved at sammenligne med en helt **anderledes** algoritme (FFT), beviser vi at optimeringen er matematisk korrekt.

---

## Hurtig Start

```matlab
% I MATLAB, naviger til Docs/Matlab/ og kør:
verify_dft
```

1. Vælg COM-port fra listen
2. MATLAB fanger automatisk 64 samples fra Arduino
3. Beregner `X = fftshift(fft(x))/N` og ekstraherer f=2kHz
4. Sammenligner med Arduinoens resultat
5. Viser PASS/FAIL og centreret spektrum (-4kHz til +4kHz)

---

## Krav

### Hardware
- Arduino Nano med firmware (`CAPTURE_SAMPLES = 1`)
- USB-forbindelse

### Software
- MATLAB (R2019b eller nyere)
- Firmware uploadet: `pio run -t upload`

---

## Brug

```matlab
verify_dft
```

Output:
```
Available ports:
  [1] COM3
  [2] COM5
  [3] COM10

Select port number: 3
```

---

## Forventet Output

```
========================================
   DFT Verification - VLF Metal Detector
========================================

Connecting to COM10...
Waiting for Arduino READY signal...
Arduino ready!
Triggering capture...
Capture complete!

Parsed 64 samples

Method: FFT with fftshift/N (DSP course convention)
Extracted bin at f = 2000 Hz (index 49)

=== VERIFICATION ===

Arduino: Optimized 4-sample DFT (only bin k=16)
MATLAB:  X = fftshift(fft(x))/N, extract f=2kHz

If results match, Arduino algorithm is mathematically correct!

         Arduino    MATLAB     Error
Re:       -11539    -11539        0
Im:        -7982     -7982        0
Mag:         221       221        0
Phase:      -145      -145        0°

*** PASS - Arduino optimization matches standard FFT! ***

Plots saved to Docs/Images/:
  - dft_samples.png
  - dft_spectrum.png
  - dft_phasor.png
  - dft_comparison.png

========================================
  DFT Verification: PASS
  (fftshift/N matches Arduino)
========================================
```

---

## Grafisk Output

### Tidsdomæne Samples
Viser de 64 ADC samples (DC fjernet):

![[../Images/dft_samples.png]]

### FFT Spektrum
Centreret magnitude spektrum ($-F_s/2$ til $+F_s/2$) med ±2kHz fremhævet:

![[../Images/dft_spectrum.png]]

> [!info] Konjugeret Symmetri
> For reelle signaler gælder $|X(-f)| = |X(f)|$.
> En cosinus ved 2kHz giver peaks ved **både** +2kHz og -2kHz:
> $$\cos(2\pi f t) = \frac{e^{j2\pi f t} + e^{-j2\pi f t}}{2}$$
> Arduino behøver kun beregne +2kHz da amplituden er identisk.

### Phasor Diagram
Sammenligning af Arduino (optimeret) og MATLAB FFT phasors:

![[../Images/dft_phasor.png]]

### Verifikations-Sammenfatning
Oversigt over metode og resultater:

![[../Images/dft_comparison.png]]

> [!note] Opdater graferne
> Kør `verify_dft` i MATLAB for at generere nye grafer med aktuelle data.

---

## Tolerancer

| Parameter | Tolerance | Beskrivelse |
|-----------|-----------|-------------|
| Magnitude | ±5 | Efter division med N |
| Fase | ±3° | Grader |

> [!tip] Forventede afvigelser
> Små afvigelser skyldes floating-point vs integer afrunding.
> Re/Im bør matche præcist da begge metoder bruger samme samples.

---

## Firmware Konfiguration

I `Code/src/main.c`:

```c
#define CAPTURE_SAMPLES 1    // Aktiver capture mode
#define UART_BAUD 115200     // Serial hastighed
```

### Serial Protokol

| Kommando | Respons | Beskrivelse |
|----------|---------|-------------|
| (connect) | `READY` | Arduino klar |
| `C` | `ACK` + data | Trigger capture |

---

## Fejlfinding

| Problem | Løsning |
|---------|---------|
| "No serial ports available" | Tjek USB-forbindelse |
| Timeout ved READY | Vent på Arduino reset, eller tjek firmware |
| Timeout ved capture | Tjek at `CAPTURE_SAMPLES = 1` i firmware |
| Parsed 0 samples | Arduino nåede ikke at sende READY - prøv igen |
| Garbage i serial | Forkert baud rate - skal være 115200 |

---

## Filer

| Fil | Placering | Beskrivelse |
|-----|-----------|-------------|
| `verify_dft.m` | `Docs/Matlab/` | Verifikationsscript (DSP konvention) |
| `dft_samples.png` | `Docs/Images/` | Tidsdomæne plot |
| `dft_spectrum.png` | `Docs/Images/` | Centreret FFT spektrum (±4kHz) |
| `dft_phasor.png` | `Docs/Images/` | Phasor diagram |
| `dft_comparison.png` | `Docs/Images/` | Resultat-sammenfatning |

---

## Matematisk Baggrund

### Arduino: Optimeret Single-Bin DFT

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

### MATLAB: DSP Kursus Konvention (62743)

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

### Hvorfor de matcher

Begge metoder beregner præcis det samme:
- $Re = \sum x[n] \cdot \cos(2\pi \cdot 16 \cdot n / 64)$
- $Im = \sum x[n] \cdot (-\sin(2\pi \cdot 16 \cdot n / 64))$

Arduino bruger bare det faktum at disse cos/sin værdier kun er +1, -1, eller 0.

Se [[../Theory/DFT Algorithm|DFT Algoritme]] for mere detaljer.

---

## Relaterede Dokumenter

- [[../Theory/DFT Algorithm|DFT Algoritme]]
- [[Test_RC_Loopback|RC Loopback Test]]
- [[Code_Review|Firmware Kodegennemgang]]

---

*Sidst opdateret: 12. Januar 2026 — Opdateret til DSP kursus konvention*
