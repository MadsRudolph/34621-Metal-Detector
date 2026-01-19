# Projekt Køreplan - VLF Metaldetektor

> [!abstract] Dokumentformål
> Omfattende projektstatus, kravanalyse og implementeringskøreplan for DTU 34621 Metaldetektor projektet.
>
> **Sidst Opdateret:** 2026-01-19
> **Kursus:** DTU 34621 - Indlejrede Systemer

---

## 1. Projektstatus Oversigt

### 1.1 Samlet Færdiggørelse

| Kategori | Færdiggørelse | Noter |
|----------|---------------|-------|
| **Software Kerne** | 95% | TX, ADC, DFT, IIR filter, klassificering |
| **Software UI** | 95% | Knapper, grafisk HUD, buzzer, jingle |
| **Hardware** | 40% | Designet, delvist bygget |
| **Integration** | 10% | Afventer hardware |
| **Samlet** | ~75% | Software feature-komplet |

### 1.2 Hvad Virker Nu

- TX signalgenerering (2 kHz firkantbølge på Pin 9)
- ADC sampling (8 kHz, auto-triggered fra Timer0)
- DFT beregning (64-sample vindue, single-bin ved 2 kHz)
- Magnitude og fase beregning
- IIR filter til stabil visning (alpha = 0.15)
- Metal klassificering (ferro/non-ferro baseret på fase)
- Start/Stop knap (D2) - toggle detektor on/off
- Kalibrering knap (D3) - gem baseline aflæsning
- Debug knap (D4) - skift mellem DFT/Debug skærm
- Grafisk HUD display med ikoner (metal type, signal styrke)
- Progress bar til signal niveau
- Buzzer feedback ved metal detektion
- Startup jingle ved opstart
- Splash screen med logo
- Modulær kodestruktur (11 moduler + include/ mappe)
- Verificeret med RC loopback test (-145 grader fase målt)

### 1.3 Hvad Mangler

**Software:**
- ~~Metaltype klassificering (ferro vs ikke-ferro)~~ ✅ Færdig
- ~~Start/Stop knap funktionalitet~~ ✅ Færdig
- ~~Kalibrering/nulstillingsknap funktionalitet~~ ✅ Færdig
- ~~IIR/FIR filter til display udjævning~~ ✅ Færdig
- ~~Tilstandsmaskine for driftstilstande~~ ✅ Færdig
- Evt. EEPROM persistens af kalibrering (valgfrit)

**Hardware:**
- TX driver H-bro (designet, delvist bygget)
- RX forstærker kredsløb (designet, delvist bygget)
- TX/RX/Bucking spoler (designet, ikke viklet)
- Fuld system strømtest

---

## 2. Færdiggjorte Funktioner

| Krav # | Navn | Beskrivelse | Verificeret | Noter |
|--------|------|-------------|-------------|-------|
| 1 | Amplitude/fase detektion | DFT beregner magnitude og fase | Ja | RC loopback testet |
| 6a | Processor design | Arduino Nano (ATmega328P) | Ja | PlatformIO projekt |
| 7 | Muligt software design | ADC med timer interrupts | Ja | Auto-trigger virker |
| 8a | Bruger interface display | OLED viser amplitude og fase | Ja | SSD1306 I2C |
| 10 | Detektions princip | VLF drift | Ja | 2 kHz detektion |
| 11 | Samplingsfrekvens | 8 kHz sampling | Ja | Timer0 verificeret |
| 12 | Detektionsfrekvens | 2 kHz TX frekvens | Ja | Oscilloskop verificeret |

---

## 3. Udestående Opgaver

### 3.1 Software - Kritisk (Prioritet 1)

| Hastighed | Opgave | Krav # | Status | Indsats |
|-----------|--------|--------|--------|---------|
| 1 | Metaltype klassificering | 2 | ✅ Færdig | Middel |
| 2 | Start/Stop knap | 9a | ✅ Færdig | Let |
| 3 | Kalibrering/nulstillingsknap | 9b | ✅ Færdig | Middel |
| 4 | Display stabilitet (basis) | 8b | ✅ Færdig | Let |

### 3.2 Software - Valgfri (Prioritet 2)

| Hastighed | Opgave | Krav # | Status | Indsats |
|-----------|--------|--------|--------|---------|
| 5 | IIR/FIR filter udjævning | 8c | ✅ Færdig | Middel |
| 6 | Kode modularisering | - | ✅ Færdig | Svær |
| 7 | Buzzer audio feedback | - | ✅ Færdig | Let |
| 8 | Grafisk HUD med ikoner | - | ✅ Færdig | Middel |
| 9 | Startup jingle | - | ✅ Færdig | Let |
| 10 | Splash screen | - | ✅ Færdig | Let |

### 3.3 Hardware - Kritisk (Prioritet 1)

| Hastighed | Opgave | Krav # | Status | Indsats |
|-----------|--------|--------|--------|---------|
| 1 | TX driver H-bro bygning | 6b | I Gang | Middel |
| 2 | RX forstærker bygning | 6b | I Gang | Middel |
| 3 | Spolevinding (TX, RX, Bucking) | 16 | Ikke Startet | Middel |
| 4 | System integration | 6b | Blokeret | Svær |
| 5 | Strømforsyning fra 9V | 4 | Ikke Startet | Let |
| 6 | Køretidstest (100 min) | 5 | Blokeret | Middel |
| 7 | Detektionsafstand test (50mm) | 3 | Blokeret | Middel |

### 3.4 Hardware - Valgfri (Prioritet 2)

| Hastighed | Opgave | Krav # | Status | Indsats |
|-----------|--------|--------|--------|---------|
| 8 | Kabinet/indkapsling | 13 | Ikke Startet | Middel |
| 9 | 3D printet spoleform | 14 | I Gang | Let |
| 10 | Koncentrisk spole samling | 15 | Blokeret | Middel |

---

## 4. Detaljeret Opgaveopdeling

### 4.1 Metaltype Klassificering (Krav 2)

| Felt | Værdi |
|------|-------|
| **Krav** | 2 - Metal type |
| **Status** | Ikke Startet |
| **Indsats** | Middel |
| **Afhængigheder** | Virkende DFT (færdig), Hardware integration |
| **Ansvarlig** | Software |

**Beskrivelse:**
Implementer fasebaseret metalklassificering for at skelne ferromagnetiske (jern/stål) fra ikke-ferromagnetiske (kobber, messing, aluminium) metaller.

**Implementation:**
1. Gem kalibrerings baseline (luft aflæsning)
2. Beregn delta fase fra baseline
3. Anvend tærskel klassificering:
   - Negativ faseskift (mere negativ) = Ferromagnetisk
   - Positiv faseskift (mindre negativ) = Ikke-ferromagnetisk
4. Vis metaltype indikator på OLED

**Kode Placering:** `main.c` - tilføj efter `DFT_Calc()`

```c
// Pseudokode
#define FERRO_THRESHOLD -10  // grader relativt til baseline
int16_t delta_phase = ang - baseline_phase;
uint8_t metal_type = (delta_phase < FERRO_THRESHOLD) ? FERRO : NON_FERRO;
```

---

### 4.2 Start/Stop Knap (Krav 9a)

| Felt | Værdi |
|------|-------|
| **Krav** | 9a - Bruger interface primære knapper |
| **Status** | Ikke Startet |
| **Indsats** | Let |
| **Afhængigheder** | Ingen |
| **Ansvarlig** | Software |

**Beskrivelse:**
Tilføj en start/stop knap til at styre detektor drift. Pt. eksisterer kun en debug skift-knap på Pin D4.

**Implementation:**
1. Udpeg en pin til Start/Stop (foreslået Pin D2 eller D3)
2. Tilføj tilstandsvariabel: `detector_running`
3. Skift tilstand ved knaptryk
4. Når stoppet: deaktiver TX, vis "STOPPET" på display
5. Når kørende: aktiver TX, vis målinger

**Hardware:** Forbind knap mellem valgt pin og GND (brug intern pull-up)

---

### 4.3 Kalibrering/Nulstillingsknap (Krav 9b)

| Felt | Værdi |
|------|-------|
| **Krav** | 9b - Bruger interface sekundære knapper |
| **Status** | Ikke Startet |
| **Indsats** | Middel |
| **Afhængigheder** | DFT virker (færdig) |
| **Ansvarlig** | Software |

**Beskrivelse:**
Tilføj kalibreringsknap der gemmer nuværende aflæsninger som baseline (nulpunkt) når detektor er i luft (intet metal i nærheden).

**Implementation:**
1. Udpeg en pin til Kalibrering (foreslået Pin D3)
2. Tilføj variable: `baseline_mag`, `baseline_phase`
3. Ved knaptryk: gem nuværende `mag` og `ang` som baseline
4. Vis værdier relativt til baseline
5. Gem i EEPROM for persistens (valgfrit)

**Kode Placering:** Tilføj til knaphåndtering i hovedløkke

```c
// Pseudokode
if (calibration_button_pressed) {
    baseline_mag = mag;
    baseline_phase = ang;
    display_message("KALIBRERET");
}
// I display: vis (mag - baseline_mag), (ang - baseline_phase)
```

---

### 4.4 IIR/FIR Filter til Display (Krav 8c)

| Felt | Værdi |
|------|-------|
| **Krav** | 8c - Forbedret display udlæsningsstabilitet |
| **Status** | Ikke Startet |
| **Indsats** | Middel |
| **Afhængigheder** | Ingen |
| **Ansvarlig** | Software |

**Beskrivelse:**
Tilføj digitalt lavpasfilter for at udjævne display aflæsninger og reducere jitter.

**Implementation (IIR enkelt-pol):**
```c
// Simpelt IIR: y[n] = alpha * x[n] + (1-alpha) * y[n-1]
#define ALPHA 0.1  // Lavere = mere udjævning, langsommere respons
static float mag_filtered = 0;
static float ang_filtered = 0;

mag_filtered = ALPHA * mag + (1 - ALPHA) * mag_filtered;
ang_filtered = ALPHA * ang + (1 - ALPHA) * ang_filtered;
```

**Afvejninger:**
- Lavere alpha = glattere men langsommere respons
- Højere alpha = hurtigere men mere støjende
- Foreslået: alpha = 0.1 til 0.2

---

### 4.5 TX Driver H-Bro Bygning (Krav 6b)

| Felt | Værdi |
|------|-------|
| **Krav** | 6b - Hardware design |
| **Status** | I Gang |
| **Indsats** | Middel |
| **Afhængigheder** | Komponent indkøb |
| **Ansvarlig** | Hardware |

**Beskrivelse:**
Byg H-bro MOSFET driver kredsløb til at drive TX spolen med 2 kHz firkantbølge.

**Komponenter:**
- 2x IRF5305PbF (P-kanal, high-side)
- 4x IRL530 (N-kanal, logic-level)
- Gate modstande, bypass kondensatorer
- TX spole (6.33 mH)
- Serie modstand (35 ohm)

**Reference:** [[TX Driver Design|TX Driver Design dokumentation]]

---

### 4.6 RX Forstærker Bygning (Krav 6b)

| Felt | Værdi |
|------|-------|
| **Krav** | 6b - Hardware design |
| **Status** | I Gang |
| **Indsats** | Middel |
| **Afhængigheder** | Komponent indkøb |
| **Ansvarlig** | Hardware |

**Beskrivelse:**
Byg modtageforstærker til at forstærke svagt RX spole signal til ADC indgang.

**Design Krav:**
- Indgang: ~1-10 mV fra RX spole
- Udgang: 0-5V område til ADC
- Forstærkning: ~100-500x
- Båndpas centreret ved 2 kHz

**Reference:** Se KiCad skematik for kredsløbsdetaljer

---

### 4.7 Spolevinding (Krav 16)

| Felt | Værdi |
|------|-------|
| **Krav** | 16 - Modtager spolens selvinduktans |
| **Status** | Ikke Startet |
| **Indsats** | Middel |
| **Afhængigheder** | Spoleforme, tråd |
| **Ansvarlig** | Hardware |

**Beskrivelse:**
Vikl TX, RX og Bucking spoler ifølge designspecifikationer.

**Specifikationer:**

| Spole | Diameter | Vindinger | Lag | Induktans | Tråd |
|-------|----------|-----------|-----|-----------|------|
| TX | 200 mm | 68 | 2 | 6.33 mH | 0.52 mm (AWG 24) |
| Bucking | 120 mm | ~35 | 1 | ~3 mH | 0.52 mm |
| RX | 80 mm | ~170 | 4 | ≥10 mH | 0.32 mm (AWG 28) |

**Reference:** [[Coil Design|Spole Design dokumentation]]

---

## 5. Kode Struktur (Implementeret)

### 5.1 Nuværende Struktur ✅

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
    ├── I2C.c, I2C.h
    ├── ssd1306.c, ssd1306.h
    └── data.h          # Font og ikon data (PROGMEM)
```

### 5.2 Modularisering Status

Koden er nu fuldt modulariseret med 11 separate moduler plus drivers.

### 5.3 Modul Oversigt

| Modul | Ansvar |
|-------|--------|
| `config.h` | Globale konstanter (F_SAMPLE, F_SIGNAL, N, tærskler) |
| `timer.c/h` | Timer0 (8kHz, TX toggle) og Timer1 (buzzer PWM) |
| `adc.c/h` | ADC auto-trigger setup |
| `dft.c/h` | Single-bin DFT akkumulering og beregning |
| `filter.c/h` | IIR lavpas filter (alpha = 0.15) |
| `detection.c/h` | Kalibrering og metal klassificering |
| `display.c/h` | Grafisk HUD, ikoner, progress bar, splash |
| `button.c/h` | Debounced knaphåndtering (D2, D3, D4) |
| `buzzer.c/h` | Tone-generering via Timer1 PWM |
| `jingle.c/h` | Startup-melodi sekvens |
| `capture.c/h` | MATLAB serial interface |

### 5.4 ISR Noter

> [!note] Volatile Variable
> Variable tilgået af ISR'er er markeret `volatile`:
> - `dft_done` flag: sat af ADC ISR, læst af main
> - `Re_buf`, `Im_buf`: skrevet af ISR, læst af main
> - `sync_flag`: delt mellem Timer0 ISR og ADC ISR

---

## 6. Foreslået Implementations Rækkefølge

### Fase 1: Færdiggør Software Funktioner (Før Hardware Integration)

| Trin | Opgave | Indsats | Test Metode |
|------|--------|---------|-------------|
| 1 | Tilføj Start/Stop knap | Let | Verificer TX skifter til/fra |
| 2 | Tilføj Kalibreringsknap | Middel | Verificer baseline lagring |
| 3 | Tilføj Metalklassificering | Middel | Test med RC loopback (faseskift) |
| 4 | Tilføj IIR filter (valgfrit) | Middel | Verificer display stabilitet |

### Fase 2: Hardware Integration

| Trin | Opgave | Indsats | Test Metode |
|------|--------|---------|-------------|
| 5 | Byg TX driver | Middel | Oscilloskop verificer 2kHz output |
| 6 | Byg RX forstærker | Middel | Oscilloskop verificer signalkæde |
| 7 | Vikl spoler | Middel | LCR meter verificer induktans |
| 8 | Forbind 9V strøm | Let | Mål strømforbrug |
| 9 | Fuld system test | Svær | Metaldetektions test |

### Fase 3: Test & Validering

| Trin | Opgave | Indsats | Test Metode |
|------|--------|---------|-------------|
| 10 | Verificer 50mm detektion | Middel | Fysisk test med Fe cylinder |
| 11 | Verificer metal diskriminering | Middel | Test Fe vs Cu/Messing/Al |
| 12 | Køretidstest (100 min) | Middel | Batteri afladningstest |
| 13 | Brugertest (læsbarhed) | Let | Lad andre læse display |

### Fase 4: Kode Oprydning (Efter Funktioner Er Færdige)

| Trin | Opgave | Indsats | Test Metode |
|------|--------|---------|-------------|
| 14 | Kode modularisering | Svær | Verificer alle funktioner stadig virker |
| 15 | Kode dokumentation | Let | Gennemgang og kommentarer |
| 16 | Endelig oprydning | Let | Fjern debug kode |

---

## 7. Nuværende Kode Problemer

### 7.1 Kritiske Problemer

| Problem | Placering | Foreslået Løsning | Prioritet |
|---------|-----------|-------------------|-----------|
| Ingen tilstandsmaskine | main.c | Tilføj detector_state enum (IDLE, RUNNING, CALIBRATING) | Høj |
| Ingen kalibrering | main.c | Tilføj baseline lagring og relativ visning | Høj |
| Ingen metalklassificering | main.c | Tilføj fase tærskel logik | Høj |
| Kun debug knap | main.c | Redesign knapper til Start/Stop | Høj |

### 7.2 Middel Problemer

| Problem | Placering | Foreslået Løsning | Prioritet |
|---------|-----------|-------------------|-----------|
| Ingen display filtrering | main.c | Tilføj IIR lavpas filter | Middel |
| ADC prescaler kommentar | main.c:87 | Verificer 64 prescaler er korrekt | Middel |
| DC offset hardkodet | main.c:39 | Kunne auto-kalibrere | Lav |

### 7.3 Lav Prioritet / Nice-to-Have

| Problem | Placering | Foreslået Løsning | Prioritet |
|---------|-----------|-------------------|-----------|
| Al kode i main.c | main.c | Modulariser (efter funktioner virker) | Lav |
| Ingen EEPROM persistens | - | Gem kalibrering i EEPROM | Lav |
| Magiske tal | main.c | Flyt til config.h | Lav |

---

## 8. Test Tjekliste

### 8.1 Før-Integrations Tests (Kun Software)

- [x] TX frekvens = 2 kHz ± 100 Hz (verificeret med oscilloskop)
- [x] Sample rate = 8 kHz ± 100 Hz (verificeret med oscilloskop)
- [x] DFT magnitude beregning virker
- [x] DFT fase beregning virker
- [x] OLED display opdaterer
- [x] Start/Stop knap skifter detektion
- [x] Kalibreringsknap gemmer baseline
- [x] Metaltype vises korrekt
- [x] IIR filter udjævner display
- [x] Buzzer giver feedback ved detektion
- [x] Grafisk HUD med ikoner virker
- [x] Startup jingle afspilles ved boot
- [x] Splash screen vises ved boot

### 8.2 Hardware Integrations Tests

- [ ] TX driver outputter 2 kHz til spole
- [ ] TX strøm inden for budget (<80 mA)
- [ ] RX forstærker outputter signal til ADC
- [ ] ADC læser gyldigt signal (ikke klippet, ikke støj)
- [ ] Bucking spole ophæver direkte kobling
- [ ] Total strøm <120 mA fra 9V

### 8.3 System Validerings Tests

- [ ] **Krav 1:** Amplitude detektion - magnitude ændres nær metal
- [ ] **Krav 1:** Fase detektion - fase ændres nær metal
- [ ] **Krav 2:** Metaltype - Fe giver anden aflæsning end Cu/Messing/Al
- [ ] **Krav 3:** Detektionsdybde - detekterer 15mm Fe cylinder ved 50mm
- [ ] **Krav 4:** Strøm - kører fra enkelt 9V batteri
- [ ] **Krav 5:** Køretid - drifter 100 min med >6V tilbage
- [ ] **Krav 8b:** Læsbarhed - flertal af brugere kan læse display
- [ ] **Krav 9a:** Start/Stop - knap starter og stopper detektion
- [ ] **Krav 9b:** Kalibrering - knap nulstiller aflæsningen

### 8.4 Efter-Modulariserings Tests

- [x] TX genererer stadig 2 kHz
- [x] ADC sampler stadig ved 8 kHz
- [x] DFT beregner stadig korrekt
- [x] Ingen timing drift eller glitches
- [x] Alle knapper virker stadig
- [x] Display opdaterer stadig korrekt
- [x] Buzzer virker korrekt
- [x] Jingle afspilles ved opstart

---

## 9. Risikovurdering

### 9.1 Høj Risiko

| Risiko | Sandsynlighed | Påvirkning | Afbødning |
|--------|---------------|------------|-----------|
| Hardware ikke klar i tide | Middel | Høj | Prioriter HW bygning, test SW med loopback |
| Metalklassificering virker ikke | Middel | Høj | Test med kendte metaller, justér tærskler empirisk |
| Strømforbrug for højt | Lav | Høj | Mål tidligt, reducer hvis nødvendigt (lavere TX strøm) |

### 9.2 Middel Risiko

| Risiko | Sandsynlighed | Påvirkning | Afbødning |
|--------|---------------|------------|-----------|
| Detektionsdybde utilstrækkelig | Middel | Middel | Optimer spole, øg TX strøm |
| ISR timing bryder under modularisering | Middel | Middel | Test efter hver ændring, hold ISR'er simple |
| Display for ustabilt | Lav | Middel | Tilføj IIR filter, øg DFT vindue |

### 9.3 Lav Risiko

| Risiko | Sandsynlighed | Påvirkning | Afbødning |
|--------|---------------|------------|-----------|
| 9V batteri spænding for lav | Lav | Lav | Brug friskt batteri, test med strømforsyning |
| I2C display fejler | Lav | Lav | Hav backup display modul |

### 9.4 Ukendt / Kræver Test

- Præcise fase tærskler for Fe vs ikke-Fe klassificering
- Støjniveauer i virkeligheden med rigtige spoler
- Bucking spole justeringspræcision påkrævet
- Temperatureffekter på kalibrering

---

## 10. Relaterede Dokumenter

- [[Coil Design|Spole Design]] - Detaljerede spolespecifikationer
- [[TX Driver Design|TX Driver Design]] - H-bro kredsløbsdesign
- [[Power Budget Analysis|Strømbudget Analyse]] - Strømforbrugsanalyse
- [[DFT Algorithm|DFT Algoritme]] - DSP algoritme dokumentation
- [[Code_Review|Firmware Kodegennemgang]] - Kodestruktur dokumentation
- [[Assembly_Guide|Samlevejledning]] - Hardware samlevejledning
- [[Testing Guide|Testguide]] - AD3, RC Loopback, MATLAB verifikation
- [[kravspecifikation.pdf|Kravspecifikation]] - Officielle krav

---

*Projekt Køreplan for DTU 34621 Metaldetektor*
*Opdateret 2026-01-19*

#køreplan #status #krav #planlægning
