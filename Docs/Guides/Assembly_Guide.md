# VLF Metaldetektor - Montagevejledning

## Arduino Nano Breadboard Ledningsføring

Denne guide viser hvordan man forbinder Arduino Nano til DTU 34621 Metaldetektor projektet.

**Nuværende Status:** Minimal testkonfiguration til kerne TX/RX/DSP test.

---

## Komponentliste (Minimal Testversion)

| Komponent | Antal | Noter |
|-----------|-------|-------|
| Arduino Nano | 1 | ATmega328P, 16MHz |
| Breadboard | 1 | Fuld størrelse anbefalet |
| SSD1306 OLED Display | 1 | 128x64, I2C interface |
| Trykknap | 1 | Debug skærm skift (D4) |
| TX Spole driver | 1 | H-bro eller transistor kredsløb |
| RX Spole forstærker | 1 | Op-amp kredsløb til spolesignal |
| 46.3kΩ modstand | 1 | Batteri spændingsdeler (R1) |
| 10kΩ modstand | 1 | Batteri spændingsdeler (R2) |
| Jumperwires | ~10 | Forskellige farver anbefalet |

### Komponenter til Senere (Ikke Brugt i Minimal Version)

| Komponent | Antal | Noter |
|-----------|-------|-------|
| Trykknap | 1 | Start/Stop (D2) |
| Piezo buzzer | 1 | Til lydtilbagemelding |

---

## Pin Forbindelser

### Arduino Nano Pinout Oversigt

```
                     +-----+
                     | USB |
                     +-----+
            +--------+-----+--------+
        TX1 | [ ]  D1|     |VIN [ ] | VIN
        RX0 | [ ]  D0|     |GND [ ] | GND
            | [ ] RST|     |RST [ ] |
            | [ ] GND|     |5V  [ ] | 5V
            | [ ]  D2|     |A7  [ ] |
            | [ ]  D3|     |A6  [ ] |
 DEBUG  [X] | [X]  D4|     |A5  [X] | SCL
            | [ ]  D5|     |A4  [X] | SDA
            | [ ]  D6|     |A3  [ ] |
            | [ ]  D7|     |A2  [ ] |
            | [ ]  D8|     |A1  [X] | BAT-MON
 TX-UD  [X] | [X]  D9|     |A0  [X] | RX-IND
            | [ ] D10|     |AREF[ ] |
 MOSI       | [ ] D11|     |3V3 [ ] |
 MISO       | [ ] D12|     |D13 [ ] | SCK
            +-------+-------+-------+

[X] = Brugt i minimal testversion
```

---

## Ledningstabel (Minimal Testversion)

| Funktion | Arduino Pin | Port | Retning | Forbindelse |
|----------|-------------|------|---------|-------------|
| **TX Signal** | D9 | PB1 | Udgang | TX spole driver indgang |
| **RX Indgang** | A0 | PC0 | Indgang | RX forstærker udgang |
| **Batteri Monitor** | A1 | PC1 | Indgang | Spændingsdeler udgang |
| **Debug Knap** | D4 | PD4 | Indgang | Knap til GND |
| **OLED SDA** | A4 | PC4 | I2C | Display SDA |
| **OLED SCL** | A5 | PC5 | I2C | Display SCL |
| **5V** | 5V | - | Strøm | Display VCC, komponenter |
| **GND** | GND | - | Strøm | Fælles stel |

### Pins Reserveret til Fremtidig Brug

| Funktion | Arduino Pin | Port | Noter |
|----------|-------------|------|-------|
| Start/Stop Knap | D2 | PD2 | Tilføjes senere |
| Buzzer | D3 | PD3 | Tilføjes senere |
| Debug Pin | D8 | PB0 | Valgfri oscilloskop |

---

## Ledningsdiagram (ASCII)

```
                                    +------------------+
                                    |   SSD1306 OLED   |
                                    |                  |
                                    | VCC  GND  SCL SDA|
                                    +--+---+----+---+--+
                                       |   |    |   |
                                       |   |    |   |
    +----------------------------------+---+----+---+-----------+
    |                                  |   |    |   |           |
    |  ARDUINO NANO                    |   |    |   |           |
    |  +-----------+                   |   |    |   |           |
    |  |           |                   |   |    |   |           |
    |  |       5V  +-------------------+   |    |   |           |
    |  |      GND  +---[FÆLLES STEL]-------+    |   |           |
    |  |       A5  +------------------------+   |   |           |
    |  |       A4  +----------------------------+   |           |
    |  |       A1  +---[BATTERI SPÆNDINGSDELER]----+           |
    |  |       A0  +---[RX FORSTÆRKER UDGANG]-------+           |
    |  |           |                                            |
    |  |       D9  +---[TX SPOLE DRIVER INDGANG]                |
    |  |           |                                            |
    |  |       D4  +---+                                        |
    |  |           |   |  +-------+                             |
    |  +-----------+   +--+ KNAP  +--[GND]   (Debug skift)      |
    |                     +-------+                             |
    +-----------------------------------------------------------+
```

---

## Trin-for-Trin Ledningsinstruktioner

### Trin 1: Placer Arduino Nano på Breadboard

1. Indsæt Arduino Nano så den strækker sig over midtergabet på breadboardet
2. Sørg for at alle pins er sat ordentligt i
3. Efterlad plads på begge sider til komponentforbindelser

### Trin 2: Strømskinner

1. Forbind Arduino **5V** til breadboard **+** skinne (rød)
2. Forbind Arduino **GND** til breadboard **-** skinne (blå)
3. Hvis du bruger begge sider, forbind strømskinnerne

### Trin 3: OLED Display (I2C)

| OLED Pin | Forbind Til |
|----------|-------------|
| VCC | 5V skinne (+) |
| GND | GND skinne (-) |
| SCL | Arduino A5 |
| SDA | Arduino A4 |

### Trin 4: TX Spole Driver

Forbind Arduino **D9** til din TX spole driver kredsløbs indgang.

Signalkarakteristika:
- Frekvens: 2 kHz firkantbølge
- Spænding: 0-5V logikniveau
- Strøm: ~20mA maks (brug driver kredsløb til spole)

### Trin 5: RX Spole Forstærker

Forbind din RX forstærker udgang til Arduino **A0**.

Signalkrav:
- Spændingsområde: 0-5V (centreret ved 2.5V anbefalet)
- Frekvens: 2 kHz (samme som TX)
- Båndbredde: Forstærker skal passere 2 kHz signal

### Trin 6: Batteri Spændingsmonitor

Byg en spændingsdeler til at måle batterispændingen sikkert via **A1**.

**Kredsløbsdiagram:**

```
VIN (9V batteri +) ──┬──> Til Arduino VIN (via spoledriver)
                     │
                R1  ─┴─  46.3kΩ
                     │
                     ├──────> Arduino A1 (ADC1)
                     │
                R2  ─┴─  10kΩ
                     │
                    GND
```

**Breadboard Layout:**

```
    9V+  ───────┬─────────────────────────
                │
           ┌────┴────┐
           │ 46.3kΩ  │  R1 (1% præcision)
           └────┬────┘
                │
                ├─────────────── A1 (jumper wire)
                │
           ┌────┴────┐
           │  10kΩ   │  R2 (brun-sort-orange)
           └────┬────┘
                │
    GND  ───────┴─────────────────────────
```

**Beregninger:**

| Parameter | Værdi | Forklaring |
|-----------|-------|------------|
| Delerforhold | 10/(46.3+10) = 0.1776 | Reducerer spænding til sikker niveau |
| Ved 9V batteri | 9 × 0.1776 = **1.60V** | Fuld opladning → ADC værdi ~328 |
| Ved 6V batteri | 6 × 0.1776 = **1.07V** | Tom → ADC værdi ~219 |
| Strømforbrug | 9V / 56.3kΩ = **0.16mA** | Minimal batteridræn |

**Monteringsinstruktioner:**

1. **Placer 46.3kΩ modstand (R1):**
   - Én ben forbindes til 9V batteriet positive pol (eller VIN skinne)
   - Anden ben til en fri række på breadboard

2. **Placer 10kΩ modstand (R2):**
   - Én ben forbindes til samme række som R1's anden ben
   - Anden ben forbindes til GND skinne

3. **Forbind til Arduino:**
   - Fra midterpunktet (hvor R1 og R2 mødes) → jumper wire til Arduino **A1**

**Vigtige Bemærkninger:**

> ⚠️ **ADC Reference:** Kredsløbet antager 5V ADC reference (AVCC).
>
> ⚠️ **Modstandstolerancer:** Brug 1% eller bedre modstande for nøjagtig aflæsning.
>
> ⚠️ **Maksimal Spænding:** Aldrig tilslut mere end 28V til VIN (giver 4.9V på A1).

**Verifikation:**

1. Mål med multimeter mellem midterpunkt og GND
2. Ved 9V batteri bør du måle ca. 1.5-1.6V
3. Display viser spænding i format "X.XV" på hovedskærm og debug skærm

**Forventet Spændingsaflæsning:**

| Strømkilde | Forventet Aflæsning | Forklaring |
|------------|---------------------|------------|
| Bench strømforsyning 9V | 9.0V | Lav indre modstand, stabil spænding |
| Frisk 9V alkaline batteri | 8.0-8.5V | Spændingsfald under belastning |
| Brugt 9V batteri | 6.5-7.5V | Øget indre modstand med alder |
| Tomt batteri | < 6.5V | Tid til udskiftning |

> **Vigtigt:** Displayet viser den *faktiske* spænding under belastning - ikke batteris tomgangsspænding!
>
> En 9V batteri målt med multimeter uden belastning kan vise 9V, men kun 7-8V når
> den forsyner kredsløbet. Dette skyldes batteris indre modstand (typisk 1-2Ω for alkaline).
>
> **Spændingsfald formel:** `V_drop = I_load × R_internal`
>
> Ved 80mA belastning og 1.5Ω indre modstand: `0.08A × 1.5Ω = 0.12V` minimum tab.
> I praksis er tabet større pga. batterikemi og alder.

**Batterityper og Forventet Ydelse:**

| Batteritype | Indre Modstand | Typisk Spænding Under Last |
|-------------|----------------|---------------------------|
| Alkaline (frisk) | 1-2Ω | 8.0-8.5V |
| Alkaline (brugt) | 3-5Ω | 7.0-7.5V |
| Zink-carbon | 3-6Ω | 7.0-8.0V |
| Lithium 9V | 0.5-1Ω | 8.5-9.0V |
| NiMH genopladeligt (8.4V) | 0.5-1Ω | 7.5-8.0V |

### Trin 7: Debug Knap (D4)

Forbind en trykknap mellem **D4** og **GND**.

```
Arduino D4 ----+---- KNAP ----+---- GND
               |              |
          (intern pull-up aktiveret i kode)
```

> **Bemærk:** Ingen ekstern modstand nødvendig - koden aktiverer intern pull-up.
> Tryk for at skifte mellem DFT og Debug skærme.

---

## Verifikationstjekliste (Minimal Testversion)

Efter ledningsføring, verificer hver forbindelse:

- [ ] **Strøm:** 5V og GND skinner forbundet
- [ ] **OLED:** Display viser "Starter..." derefter DFT skærm
- [ ] **Debug Knap:** Tryk D4 knap - skærm skifter til DEBUG
- [ ] **TX Signal:** Oscilloskop på D9 viser 2 kHz firkantbølge
- [ ] **RX Indgang:** A0 modtager signal fra RX forstærker
- [ ] **Batteri Monitor:** Display viser korrekt batterispænding (sammenlign med multimeter)
- [ ] **Display Værdier:** Re, Im, Mag, Fase værdier opdateres på skærm

---

## Forventet Display Output

Tryk på **D4 knappen** for at skifte mellem to skærme:

**Skærm 1: DFT Resultater**
```
=== DFT ===

Re:   <værdi>
Im:   <værdi>

Mag:  <værdi>
Fase: <værdi> grader
```

**Skærm 2: Debug Info**
```
---- DEBUG ----
ADC:  <rå værdi>
Mag:  <værdi>
Fase: <værdi>
Cal: M<mag> A<ang>
dM:<delta> dA:<delta>
RUN CAL 8.5V
----------------
```

| Værdi | Beskrivelse |
|-------|-------------|
| **Re/Im** | DFT komponenter (kan være negative) |
| **Mag** | Magnitude = sqrt(Re² + Im²) / 16 |
| **Fase** | Fasevinkel i grader |
| **ADC** | Nuværende rå ADC værdi (0-1023) |
| **X.XV** | Aktuel batterispænding (vises på begge skærme) |

---

## Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Intet display | I2C ledningsføring forkert | Tjek A4/A5 forbindelser, verificer I2C adresse |
| Display viser vrøvl | Forkert I2C adresse | Prøv 0x7A i stedet for 0x78 |
| Intet TX signal | Timer kører ikke | Tjek sei() kaldes, verificer D9 forbindelse |
| Re/Im altid 0 | Intet RX signal | Verificer RX forstærker udsender 0-5V på A0 |
| Re/Im altid samme fortegn | DC offset forkert | ADC bør være centreret ved ~512 |
| Fase sidder fast på 0 eller 90 | Ingen signalvariation | Tjek spoleforbindelser |
| Batteri viser 0.0V | Spændingsdeler ikke forbundet | Tjek 46.3k/10k modstande og A1 forbindelse |
| Batteri viser forkert spænding | Forkerte modstandsværdier | Mål modstande med multimeter, verificer 46.3k og 10k |
| Batteri spænding ustabil | Løs forbindelse | Tjek alle forbindelser i spændingsdeler kredsløb |

---

## Sikkerhedsnoter

1. **Overskrid ikke 5V** på nogen indgangspin
2. **Træk ikke mere end 20mA** fra nogen enkelt pin
3. **Brug driver kredsløb** til spoler - driv ikke direkte fra pins
4. **Afbryd strøm** før ledningsændringer

---

## Fremtidige Tilføjelser

Når klar til at tilføje UI funktioner, forbind:

| Komponent | Pin | Noter |
|-----------|-----|-------|
| Start/Stop Knap | D2 til GND | Intern pull-up i kode |
| Buzzer (+) | D3 | Negativ til GND |
| Debug Pin | D8 | Til oscilloskop timing |

---

## Reference

- ATmega328P Datasheet: Rev. 7810D-AVR-01/15
- Arduino Nano Pinout: https://www.arduino.cc/en/uploads/Main/ArduinoNanoManual23.pdf
