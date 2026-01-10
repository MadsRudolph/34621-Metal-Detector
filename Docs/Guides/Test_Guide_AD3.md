# Metaldetektor Testguide - Analog Discovery 3

## Formål

Test metaldetektorens firmware uden spoler ved at bruge Analog Discovery 3 til at:
1. **Måle** TX-udgangssignalet på Pin D9
2. **Generere** et testsignal ind på Pin A0 (simulerer RX-spole)
3. **Verificere** at DFT-behandlingen fungerer korrekt

---

## Nødvendigt Udstyr

| Udstyr | Noter |
|--------|-------|
| Arduino Nano | Programmeret med minimal test-firmware |
| Analog Discovery 3 | Med WaveForms software |
| Breadboard | Med Nano og OLED forbundet |
| Jumperwires | Til AD3 forbindelser |

---

## AD3 Pin Reference

```
Analog Discovery 3 Connector:
┌─────────────────────────────────────┐
│  1+  1-  2+  2-  W1  W2  T1  T2 ... │
│  ●   ●   ●   ●   ●   ●   ●   ●      │
│  Scope Ch1  Scope Ch2  Wavegen      │
│                                     │
│  GND GND GND ...                    │
│  ●   ●   ●                          │
└─────────────────────────────────────┘

1+/1- = Oscilloskop Kanal 1 (differentiel)
2+/2- = Oscilloskop Kanal 2 (differentiel)
W1    = Bølgegenerator 1 udgang
W2    = Bølgegenerator 2 udgang
GND   = Stel (flere pins tilgængelige)
```

---

## Test Setup Ledningsføring

### Forbindelser

| AD3 Pin | Arduino Pin | Formål |
|---------|-------------|--------|
| **1+** (orange) | D9 | Mål TX-udgang |
| **1-** (orange/hvid) | GND | Scope Ch1 reference |
| **W1** (gul) | A0 | Injicer testsignal |
| **GND** (sort) | GND | Fælles stel |

### Ledningsdiagram

```
    ANALOG DISCOVERY 3                    ARDUINO NANO
    ┌──────────────────┐                  ┌──────────────┐
    │                  │                  │              │
    │  1+ (orange) ────┼──────────────────┼── D9 (TX)    │
    │  1- (org/hvid) ──┼────┐             │              │
    │                  │    │             │              │
    │  W1 (gul) ───────┼────┼─────────────┼── A0 (RX)    │
    │                  │    │             │              │
    │  GND (sort) ─────┼────┴─────────────┼── GND        │
    │                  │                  │              │
    └──────────────────┘                  └──────────────┘
```

> **Vigtigt:** Forbind altid GND først før signalledninger.

---

## Test 1: Verificer TX-Udgang (2 kHz Firkantbølge)

### Mål
Bekræft at Pin D9 udsender en 2 kHz firkantbølge.

### WaveForms Opsætning

1. Åbn **WaveForms** software
2. Klik på **Scope** for at åbne oscilloskopet

### Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret |
| Kobling | DC |
| Område | 0-5V (eller Auto) |
| Tid/Div | 200 µs |
| Trigger | Ch1, Stigende flanke, 2.5V |

### Procedure

1. Tænd Arduino Nano
2. Klik **Run** i WaveForms Scope
3. Observer bølgeformen

### Forventet Resultat

```
     5V ┤ ┌───┐   ┌───┐   ┌───┐   ┌───┐
        │ │   │   │   │   │   │   │   │
        │ │   │   │   │   │   │   │   │
     0V ┤─┘   └───┘   └───┘   └───┘   └───
        └─────────────────────────────────
          |<-250µs->|
          |<---500µs (én periode)--->|
```

| Måling | Forventet Værdi | Tolerance |
|--------|-----------------|-----------|
| Frekvens | 2.000 kHz | ±1% |
| Periode | 500 µs | ±5 µs |
| Duty Cycle | 50% | ±2% |
| Amplitude | 5V (0 til 5V) | |

### Brug Målinger

I WaveForms Scope:
1. Klik **View** → **Measurements**
2. Tilføj: Frequency, Period, Duty Cycle, Amplitude

---

## Test 2: Injicer Testsignal og Verificer DFT

### Mål
Send en kendt 2 kHz sinusbølge ind på A0 og verificer at DFT beregner korrekt magnitude og fase.

### WaveForms Opsætning

1. Åbn **Wavegen** værktøj
2. Konfigurer Bølgegenerator 1 (W1)

### Wavegen Indstillinger

| Parameter | Værdi | Noter |
|-----------|-------|-------|
| Kanal | W1 | Gul ledning |
| Type | Sinus | Start med sinusbølge |
| Frekvens | 2000 Hz | Match TX frekvens |
| Amplitude | 2 V | Peak-to-peak |
| Offset | 2.5 V | Centrer ved mid-skala ADC |
| Symmetri | 50% | Normal sinus |

Dette producerer et signal fra **1.5V til 3.5V**, centreret ved 2.5V (ADC værdi ~512).

### Procedure

1. Konfigurer Wavegen som ovenfor
2. Klik **Run** på Wavegen
3. Observer OLED displayet på Arduino

### Forventet Display Output

Tryk på **D4 knappen** for at skifte mellem to skærme:

**Skærm 1: DFT Resultater**
```
=== DFT ===

Re:   <ikke-nul>
Im:   <ikke-nul>

Mag:  <værdi>
Fase: <vinkel> grader
```

**Skærm 2: Debug Info**
```
=== DEBUG ===

ADC:  512        (bør svinge med signal)
```

### Forståelse af Resultaterne

Med en 2 kHz sinusbølge input synkroniseret til TX:

| Display Værdi | Forventet Opførsel |
|---------------|-------------------|
| **Re** | Ikke-nul, varierer med fasejustering |
| **Im** | Ikke-nul, varierer med fasejustering |
| **Mag** | Proportional med input amplitude (~60-120 for 2Vpp) |
| **Fase** | Afhænger af faseforhold til TX |

---

## Test 3: Faseopførsel (Vigtigt!)

### Forståelse af Fasedrift

Når du tester med AD3 wavegen, vil du måske bemærke at faseaflæsningen er "over det hele" selv med korrekt ledningsføring. **Dette er forventet opførsel!**

**Grundårsag:** AD3 wavegen og Arduino kører fra uafhængige krystaloscillatorer. Selv når begge er sat til 2000 Hz, vil de have lidt forskellige faktiske frekvenser:

| Kilde | Indstillet Frekvens | Faktisk Frekvens |
|-------|---------------------|------------------|
| Arduino TX | 2000 Hz | ~2001.8 Hz |
| AD3 Wavegen | 2000 Hz | ~2001.6 Hz |

En forskel på bare 0.2 Hz får fasen til at rotere gennem 360° hvert 5. sekund!

**Dette bekræfter at din fasedetektion fungerer korrekt** - den sporer nøjagtigt det skiftende faseforhold mellem to usynkroniserede signaler.

### Metode 1: Verificer Faserotation (Bekræfter System Virker)

1. Sæt Wavegen til præcis **2000 Hz**
2. Se Fase-værdien på displayet
3. **Forventet:** Fasen roterer langsomt gennem -180° til +180°
4. Rotationshastighed afhænger af frekvensforskel (~5-10 sek per cyklus typisk)

Hvis fasen roterer jævnt: **BESTÅET** - fasedetektion virker!

### Metode 2: Bevidst Frekvensoffset

1. Sæt Wavegen til **2001 Hz** (1 Hz offset)
2. Fasen bør rotere gennem 360° hvert sekund
3. Sæt Wavegen til **2010 Hz** (10 Hz offset)
4. Fasen bør rotere gennem 360° hvert 0.1 sekund (fremstår tilfældig)

### Metode 3: Synkroniseret Test (Stabil Fase)

For at få en stabil faseaflæsning skal du synkronisere AD3 til Arduino:

**Mulighed A: Ekstern Trigger**
1. Forbind Arduino D9 (TX) til AD3 Trigger input (T1)
2. I WaveForms Wavegen, sæt trigger source til "External 1"
3. Dette synkroniserer wavegen output til Arduino timing

**Mulighed B: Brug Arduino TX som Reference**
1. Forbind en ledning fra D9 til en anden AD3 scope kanal
2. I Scope, trigger på TX signalet
3. Observer at det injicerede signals fase er stabil relativt til TX

### Hvorfor Dette Er Vigtigt for Metaldetektion

I den rigtige metaldetektor:
- TX-spole skaber magnetfeltet
- RX-spole modtager responsen
- Begge signaler kommer fra det SAMME fysiske system
- Fase er naturligt synkroniseret (bestemt af metalegenskaber)

AD3 test-opsætningen har uafhængige clocks, så fasen drifter. Dette er en begrænsning ved test-opsætningen, ikke firmwaren.

---

## Test 4: Amplitudetest

### Mål
Verificer at magnitude skalerer med input amplitude.

### Procedure

1. Sæt Wavegen frekvens til 2000 Hz, offset 2.5V
2. Registrer magnitude for forskellige amplituder:

| Wavegen Amplitude (Vpp) | Signalområde | Forventet Trend |
|-------------------------|--------------|-----------------|
| 0.5 V | 2.25V - 2.75V | Lav Mag |
| 1.0 V | 2.0V - 3.0V | Medium Mag |
| 2.0 V | 1.5V - 3.5V | Høj Mag |
| 4.0 V | 0.5V - 4.5V | Meget Høj Mag |

> **Advarsel:** Overskrid ikke 5V total (offset + amplitude/2 < 5V)

### Forventet Resultat

Magnitude bør stige omtrent lineært med input amplitude.

---

## Test 5: Frekvensrespons

### Mål
Verificer at DFT kun responderer stærkt på 2 kHz signalet.

### Procedure

1. Sæt Wavegen amplitude til 2Vpp, offset 2.5V
2. Sweep frekvens og registrer magnitude:

| Frekvens | Forventet Magnitude |
|----------|---------------------|
| 500 Hz | Meget lav (nær nul) |
| 1000 Hz | Lav |
| 1500 Hz | Lav |
| **2000 Hz** | **Maksimum** |
| 2500 Hz | Lav |
| 3000 Hz | Lav |
| 4000 Hz | Meget lav |

### Forventet Resultat

DFT bør vise en stærk top ved 2 kHz og dæmpe andre frekvenser. Dette er "lock-in" effekten af single-bin DFT.

---

## Test 6: Støjafvisning

### Mål
Verificer at DFT afviser signaler der ikke er ved 2 kHz.

### Procedure

1. Sæt Wavegen til **1000 Hz** sinusbølge, 2Vpp, 2.5V offset
2. Observer magnitude (bør være lav)
3. Skift til **2000 Hz**
4. Observer magnitude (bør være høj)

Dette demonstrerer DFT's smalbåndsfiltrerende effekt.

---

## Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Intet TX signal | Firmware kører ikke | Tjek upload, verificer sei() kaldes |
| TX forkert frekvens | Timer0 config forkert | Verificer OCR0A = 249, prescaler = 8 |
| Mag altid 0 | Intet signal på A0 | Tjek W1 forbindelse, verificer Wavegen kører |
| Mag altid samme | Kun DC, ingen AC | Tjek Wavegen offset er centreret ved 2.5V |
| **Fase "over det hele"** | **Normalt! Uafhængige clocks** | **Se Test 3 - dette bekræfter fase virker!** |
| Fase sidder fast på én værdi | Intet AC signal | Tjek wavegen kører, amplitude > 0 |
| Display opdaterer ikke | DFT_done ikke sat | Tjek ADC ISR kører |

---

## Hurtig Reference: WaveForms Indstillinger

### Scope (Måling af TX)
```
Kanal 1: TIL, DC kobling
Område: 5V
Tid: 200 µs/div
Trigger: Ch1, Stigende, 2.5V
```

### Wavegen (Simulering af RX)
```
W1: Sinus
Frekvens: 2000 Hz
Amplitude: 2 V (peak-to-peak)
Offset: 2.5 V
```

---

## Noter

- AD3 Wavegen udgangsimpedans er 50Ω, hvilket er fint for højimpedans ADC input
- Hold ledninger korte for at minimere støjopsamling
- Hvis du bruger begge scope kanaler, forbind Ch2 til A0 for at se det injicerede signal
- 8 kHz samplingen skaber noget aliasing - kun 2 kHz signaler vil blive korrekt målt
