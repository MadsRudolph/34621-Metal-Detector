# H-Bridge Forstærker Måleguide

> [!abstract] Dokumentformål
> Måleguide til karakterisering af H-bro forstærkeren med Analog Discovery 3.
> Bruger de dedikerede målepunkter i skemaerne til systematisk verifikation.

---

## Indholdsfortegnelse

1. [[#1. Oversigt]]
2. [[#2. Udstyr]]
3. [[#3. Målepunkter]]
4. [[#4. Forbindelser]]
5. [[#5. Test 1 - Deadtime Verifikation]]
6. [[#6. Test 2 - Gate Drive Signaler]]
7. [[#7. Test 3 - Half-Bridge Output]]
8. [[#8. Test 4 - Differentielt Output]]
9. [[#9. Test 5 - LC Tank Strøm]]
10. [[#10. Test 6 - RX Spole Respons]]
11. [[#11. Fejlfinding]]
12. [[#12. Relaterede Dokumenter]]

---

## 1. Oversigt

### 1.1 Formål

Denne guide beskriver systematisk verifikation af H-bro forstærkeren ved hjælp af de dedikerede målepunkter i kredsløbet. Testene verificerer:

1. **Deadtime kredsløb** - Ikke-overlappende PWM signaler
2. **Gate drive** - Bootstrap driver funktion
3. **Half-bridge output** - Switching ved midtpunkt
4. **Differentielt output** - Fuld H-bro spænding
5. **LC tank strøm** - Sinusformet strøm gennem TX spole
6. **RX spole respons** - Metaldetektion verifikation

### 1.2 Anbefalet Testrækkefølge

```text
Deadtime ──▶ Gate Drive ──▶ Half-Bridge ──▶ Diff. Output ──▶ LC Tank ──▶ RX Respons
```

> [!warning] Start med lav spænding
> Ved første test, brug lavere forsyningsspænding (f.eks. 5V i stedet for 9V) for at beskytte komponenter ved eventuelle fejl.

---

## 2. Udstyr

| Udstyr | Noter |
|--------|-------|
| Analog Discovery 3 | Med WaveForms software |
| Oscilloskop probes | 10:1 anbefales til gate signaler |
| Jumperwires | Til målepunkt forbindelser |
| DC forsyning | 9V til H-bro, 5V til logik |
| Multimeter | Til DC verifikation |

### 2.1 AD3 Pin Reference

```text
Analog Discovery 3 Connector:
┌─────────────────────────────────────────────────┐
│  1+  1-  2+  2-  W1  W2  T1  T2  T3  ...  GND  │
│  ●   ●   ●   ●   ●   ●   ●   ●   ●        ●   │
│  Scope Ch1  Scope Ch2  Wavegen   Triggers      │
└─────────────────────────────────────────────────┘

1+/1- = Oscilloskop Kanal 1 (orange/orange-hvid)
2+/2- = Oscilloskop Kanal 2 (blå/blå-hvid)
W1    = Bølgegenerator 1 udgang (gul)
GND   = Stel (sort)
```

---

## 3. Målepunkter

### 3.1 Venstre H-Bro (V_Meas1 Connector)

| Pin | Signal | Beskrivelse |
|-----|--------|-------------|
| 1 | `Meas_V_Out` | Venstre half-bridge output |
| 2 | `Meas_GATE_M3` | Low-side gate (Q3) |
| 3 | `Meas_GATE_M1` | High-side gate (Q1) |

### 3.2 Højre H-Bro (Meas_H1 Connector)

| Pin | Signal | Beskrivelse |
|-----|--------|-------------|
| 1 | `Meas_H_Out` | Højre half-bridge output |
| 2 | `Meas_GATE_M4` | Low-side gate (Q4) |
| 3 | `Meas_GATE_M2` | High-side gate (Q2) |

### 3.3 Deadtime Kredsløb (Dead_Time_Outputs1 Connector)

| Pin | Signal | Beskrivelse |
|-----|--------|-------------|
| 1 | `PWM1` | Komparator output 1 |
| 2 | `PWM1_NOT` | Inverteret PWM1 |
| 3 | `PWM2` | Komparator output 2 |
| 4 | `PWM2_NOT` | Inverteret PWM2 |

### 3.4 Input (Dead_Time_Inputs1 Connector)

| Pin | Signal | Beskrivelse |
|-----|--------|-------------|
| 1 | `PWM_In` | PWM input fra MCU |
| 2 | `+5V` | Logik forsyning |
| 3 | `GND` | Stel |

### 3.5 Blodiagram

```text
                    ┌─────────────────┐
                    │  DEADTIME PCB   │
                    │                 │
    PWM_In ────────▶│  PWM1 ─────────▶├──────▶ Meas_GATE_M1 (Q1)
                    │  PWM1_NOT ─────▶├──────▶ Meas_GATE_M3 (Q3)
                    │                 │        ├──▶ Meas_V_Out
                    │  PWM2 ─────────▶├──────▶ Meas_GATE_M2 (Q2)
                    │  PWM2_NOT ─────▶├──────▶ Meas_GATE_M4 (Q4)
                    │                 │        ├──▶ Meas_H_Out
                    └─────────────────┘
                                              │         │
                                              └────┬────┘
                                                   │
                                              LC Tank Input
                                            (Diff: V_Out - H_Out)
```

---

## 4. Forbindelser

### 4.1 Generelle Retningslinjer

> [!important] Stel Reference
> Forbind altid AD3 GND til kredsløbets GND før andre forbindelser.

| AD3 Pin | Farve | Typisk Brug |
|---------|-------|-------------|
| 1+ | Orange | Primær måling |
| 1- | Orange/hvid | GND reference |
| 2+ | Blå | Sekundær måling |
| 2- | Blå/hvid | GND reference |
| GND | Sort | Fælles stel |

### 4.2 Probe Indstillinger

| Signaltype | Probe | Spændings­område |
|------------|-------|------------------|
| Gate signaler | 10:1 | 0-15V |
| PWM logik | 1:1 | 0-5V |
| Bridge output | 10:1 | 0-9V |

---

## 5. Test 1 - Deadtime Verifikation

### 5.1 Formål

Verificer at deadtime kredsløbet genererer ikke-overlappende PWM signaler med korrekt deadtime.

### 5.2 Forbindelser

| AD3 Pin | Målepunkt | Signal |
|---------|-----------|--------|
| 1+ | Dead_Time_Outputs1 pin 1 | PWM1 |
| 2+ | Dead_Time_Outputs1 pin 2 | PWM1_NOT |
| GND | GND | Stel |

### 5.3 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, DC kobling |
| Kanal 2 | Aktiveret, DC kobling |
| Område | 0-5V |
| Tid/Div | 50 µs |
| Trigger | Ch1, Stigende, 2.5V |
| Sample Rate | ≥10 MS/s |

### 5.4 Forventet Resultat

| Måling | Forventet | Tolerance |
|--------|-----------|-----------|
| PWM1 frekvens | 2.000 kHz | ±1% |
| Duty cycle | 50% | ±2% |
| Deadtime | ~15 µs | ±5 µs |
| High niveau | 5V | ±0.2V |
| Low niveau | 0V | ±0.2V |

### 5.5 Verifikationsmetode

1. Mål tiden fra PWM1 faldende flanke til PWM1_NOT stigende flanke
2. Mål tiden fra PWM1_NOT faldende flanke til PWM1 stigende flanke
3. Begge tider skal være ~15µs (deadtime)

```text
PWM1      ────┐     ┌─────────────┐     ┌────
             │     │             │     │
             └─────┘             └─────┘

PWM1_NOT  ────────┐     ┌───────────┐     ┌──
                  │     │           │     │
             ─────┘     └───────────┘     └──
                  │◄───►│
                  Deadtime (~15µs)
```

### 5.6 Måleresultater

#### Deadtime Oversigt

![[../Images/Deadtime_Blue-PWM1_not_Yellow-PWM1.png]]

**Observation:** PWM1 (gul) og PWM1_NOT (blå) viser komplementære signaler. Når PWM1 er høj, er PWM1_NOT lav og omvendt. Signalerne overlapper ikke - deadtime kredsløbet fungerer korrekt.

#### Deadtime Detalje (5 µs/div)

![[../Images/Deadtime_Blue-PWM1_not_Yellow-PWM1___5_us.png]]

**Observation:** Ved højere tidsopløsning ses deadtime tydeligt mellem flankerne. Der er en kort periode hvor begge signaler er lave, hvilket forhindrer shoot-through i H-broen.

### 5.7 Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Ingen deadtime | RC tidskonstant forkert | Tjek R/C værdier i deadtime kredsløb |
| Overlap mellem signaler | Komparator offset | Justér threshold spændinger |
| Ustabilt signal | Dårlig stel | Brug kortere GND forbindelse |

---

## 6. Test 2 - Gate Drive Signaler

### 6.1 Formål

Verificer at bootstrap gate driver leverer korrekte Vgs niveauer til high-side og low-side MOSFETs.

### 6.2 Forbindelser

| AD3 Pin | Målepunkt | Signal |
|---------|-----------|--------|
| 1+ | V_Meas1 pin 3 | Meas_GATE_M1 (high-side) |
| 2+ | V_Meas1 pin 2 | Meas_GATE_M3 (low-side) |
| GND | GND | Stel |

### 6.3 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, DC kobling, 10:1 probe |
| Kanal 2 | Aktiveret, DC kobling, 10:1 probe |
| Område | -5V til +15V |
| Tid/Div | 100 µs |
| Trigger | Ch2, Stigende, 2V |

### 6.4 Forventet Resultat

| Måling | Forventet | Tolerance |
|--------|-----------|-----------|
| Low-side Vgs (høj) | ~9V | ±1V |
| Low-side Vgs (lav) | ~0V | ±0.5V |
| High-side Vgs (høj) | ~9V over source | ±1V |
| Komplementær operation | 180° fase | - |
| Ingen overlap | Deadtime synlig | ~15µs |

### 6.5 Gate Drive Timing

```text
GATE_M3   ────┐           ┌─────────────┐
(Low-side)    │           │             │
              └───────────┘             └───────

GATE_M1   ────────┐           ┌───────────┐
(High-side)       │           │           │
              ────┘           └───────────┘
                  │◄─────────►│
                   Komplementær
```

> [!tip] Bootstrap Verifikation
> High-side gate spændingen skal være relativ til source (som svinger med output). Brug scope math: CH1 - output for at se Vgs direkte.

### 6.6 Måleresultater

![[../Images/Gate_drive_signal_left_Yellow-Highside_Blue-lowside.png]]

**Observation:**
- **Gul (CH1):** High-side gate (Meas_GATE_M1) - svinger fra 0V til ~15V relativt til GND. Dette er korrekt da bootstrap driveren løfter gate spændingen over source.
- **Blå (CH2):** Low-side gate (Meas_GATE_M3) - svinger fra 0V til ~9V.
- Signalerne er komplementære med synlig deadtime mellem skift.
- High-side gate når ~15V fordi source (switching node) er på ~6-9V når MOSFET er tændt, og gate skal være ~9V over source.

### 6.7 Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| High-side Vgs for lav | Bootstrap kondensator ikke opladet | Verificer low-side switch aktivt |
| Gate signal mangler | Driver defekt | Tjek driver IC forsyning |
| Langsom flanke | Gate modstand for høj | Verificer gate modstand værdi |

---

## 7. Test 3 - Half-Bridge Output

### 7.1 Formål

Verificer switching waveform ved half-bridge midtpunkterne og kontroller 180° faseforskydning.

### 7.2 Forbindelser

| AD3 Pin | Målepunkt | Signal |
|---------|-----------|--------|
| 1+ | V_Meas1 pin 1 | Meas_V_Out (venstre) |
| 2+ | Meas_H1 pin 1 | Meas_H_Out (højre) |
| GND | GND | Stel |

### 7.3 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, DC kobling, 10:1 probe |
| Kanal 2 | Aktiveret, DC kobling, 10:1 probe |
| Område | -2V til +12V |
| Tid/Div | 100 µs |
| Trigger | Ch1, Stigende, 4.5V |

### 7.4 Forventet Resultat

| Måling | Forventet | Tolerance |
|--------|-----------|-----------|
| Output swing | 0V til 9V | ±0.5V |
| Frekvens | 2.000 kHz | ±1% |
| Faseforskydning | 180° | ±5° |
| Rise time | <100 ns | - |
| Fall time | <100 ns | - |

### 7.5 Faseforskydning Verifikation

```text
Meas_V_Out  ────┐           ┌─────────────┐
(Venstre)       │           │             │
                └───────────┘             └───

Meas_H_Out  ───────────┐           ┌──────────
(Højre)                │           │
                ───────┘           └──────────
                       │◄─────────►│
                         180° fase
                        (250µs ved 2kHz)
```

### 7.6 Måleresultater

![[../Images/output_yellow-Vout_Blue-Hout.png]]

**Observation:**
- **Gul (CH1):** Meas_V_Out (venstre half-bridge) - svinger fra 0V til ~10V.
- **Blå (CH2):** Meas_H_Out (højre half-bridge) - svinger fra 0V til ~10V.
- **180° faseforskydning:** Når gul er høj, er blå lav og omvendt - præcis som forventet.
- Rene firkantbølger med minimal ringing på flankerne.
- Frekvens ~2kHz verificeret via tidsbasen.

### 7.7 Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Output når ikke 9V | MOSFET ikke fuldt tændt | Tjek gate drive |
| Output når ikke 0V | MOSFET ikke fuldt slukket | Tjek deadtime |
| Forkert fase | Signaler forbyttet | Tjek PWM routing |
| Ringing på flanker | Parasitisk induktans | Tilføj snubber |

---

## 8. Test 4 - Differentielt Output

### 8.1 Formål

Mål den fulde H-bro udgangsspænding der driver LC tanken.

### 8.2 Forbindelser

Samme som Test 3:

| AD3 Pin | Målepunkt | Signal |
|---------|-----------|--------|
| 1+ | V_Meas1 pin 1 | Meas_V_Out |
| 2+ | Meas_H1 pin 1 | Meas_H_Out |
| GND | GND | Stel |

### 8.3 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret |
| Kanal 2 | Aktiveret |
| **Math** | CH1 - CH2 |
| Område | -12V til +12V |
| Tid/Div | 200 µs |
| Trigger | Math, Stigende, 0V |

### 8.4 Aktivering af Math i WaveForms

1. Åbn **Scope** instrumentet
2. Klik på **Math** fanen
3. Vælg **Custom** og indtast: `C1 - C2`
4. Aktivér Math kanalen

### 8.5 Forventet Resultat

| Måling | Forventet | Tolerance |
|--------|-----------|-----------|
| Positiv amplitude | +9V | ±0.5V |
| Negativ amplitude | -9V | ±0.5V |
| Peak-to-peak | 18V | ±1V |
| Frekvens | 2.000 kHz | ±1% |
| Bølgeform | Firkant | - |

### 8.6 Differentielt Output Waveform

```text
         +9V  ─ ─ ─ ┬─────────────┬ ─ ─ ─ ─ ─ ┬───────────
                    │             │           │
          0V  ─ ─ ─ ┼ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─ ─
                    │             │           │
         -9V  ─────┬┘             └───────────┘

                   │◄────────────►│
                      250µs (½T)
```

### 8.7 Måleresultater

![[../Images/Differential_output.png]]

**Observation:**
- **Math (C1-C2):** Differentielt output viser ±9V firkantbølge.
- **Positiv peak:** +9V ✓
- **Negativ peak:** -9V ✓
- **Peak-to-peak:** ~18V ✓
- Ren firkantbølge centreret omkring 0V.
- Dette er spændingen der driver LC tanken og bekræfter at begge half-bridges arbejder korrekt med 180° faseforskydning.

### 8.8 Fejlfinding

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Asymmetrisk amplitude | Ubalanceret forsyning | Tjek 9V på begge sider |
| Kun ±4.5V | En side switcher ikke | Tjek begge half-bridges |
| DC offset | Duty cycle mismatch | Verificer 50% duty |

---

## 9. Test 5 - LC Tank Strøm

### 9.1 Formål

Verificer sinusformet strøm gennem TX spolen. Denne test er valgfri og kræver strømprobe eller shunt modstand.

### 9.2 Metode A: Strømprobe

| AD3 Pin | Forbindelse | Signal |
|---------|-------------|--------|
| 1+ | Strømprobe output | Spolestrøm |
| GND | GND | Stel |

### 9.3 Metode B: Shunt Modstand

Indsæt en lille shunt modstand (f.eks. 0.1Ω) i serie med LC tanken.

| AD3 Pin | Forbindelse | Signal |
|---------|-------------|--------|
| 1+ | Over shunt (høj side) | Spændingsfald |
| 1- | Over shunt (lav side) | Reference |

**Strøm beregning:** $I = V_{shunt} / R_{shunt}$

### 9.4 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, AC kobling |
| Område | Afhængig af probe/shunt |
| Tid/Div | 200 µs |
| Trigger | Ch1, Stigende |

### 9.5 Forventet Resultat

| Måling | Forventet | Noter |
|--------|-----------|-------|
| Bølgeform | Sinusformet | Fra LC resonans |
| Frekvens | 2.000 kHz | Matches TX frekvens |
| Amplitude | Afhænger af Q | Typisk flere hundrede mA |

### 9.6 LC Resonans Verifikation

```text
Spænding (firkant):     Strøm (sinus):
    ┌───┐                    ╱╲
    │   │                  ╱    ╲
────┘   └────           ──╱──────╲──
                           ╲    ╱
                            ╲╱
```

> [!info] Resonans Princip
> LC tankens høje Q-faktor filtrerer firkantbølgen til en sinusbølge. Kun grundtonen ved 2kHz passerer effektivt.

### 9.7 Måleresultater

![[../Images/LC_Tank_current.png]]

**Observation:**
- **Sinusformet bølgeform:** LC tanken filtrerer firkantbølgen til en sinusstrøm ✓
- **Amplitude:** ~±25mV over 0.1Ω shunt → ~250mA peak strøm
- **Switching spikes:** De skarpe transienter ved hver periode er normale og skyldes H-bro switching. De påvirker ikke metaldetektorens funktion da DFT kun ser på 2kHz grundtonen.
- **Frekvens:** 2kHz matcher TX frekvensen ✓

**Strømberegning:**
$$I_{peak} = \frac{V_{shunt}}{R_{shunt}} = \frac{25\text{mV}}{0.1\Omega} \approx 250\text{mA}$$

> [!success] Test Godkendt
> Den sinusformede strøm bekræfter at LC tanken resonerer korrekt ved 2kHz og at H-bro forstærkeren driver TX spolen som forventet.

---

## 10. Test 6 - RX Spole Respons

### 10.1 Formål

Verificer at RX spolen detekterer metal ved at måle amplitudeændringen når metal bringes tæt på spolerne. Dette demonstrerer metaldetektorens grundlæggende funktionsprincip.

### 10.2 Forbindelser

| AD3 Pin | Forbindelse | Signal |
|---------|-------------|--------|
| 1+ (orange) | RX spole signal (før forstærker) | Induceret spænding |
| 1- (orange/hvid) | RX spole GND | Reference |
| GND | Circuit GND | Stel |

### 10.3 WaveForms Scope Indstillinger

| Parameter | Værdi |
|-----------|-------|
| Kanal 1 | Aktiveret, AC kobling |
| Område | 50-100 mV/div |
| Tid/Div | 200 µs |
| Trigger | Ch1, Auto |

### 10.4 Testprocedure

1. **Baseline måling:** Tag screenshot uden metal i nærheden af spolerne
2. **Metal måling:** Hold stål (eller andet metal) tæt på TX/RX spolerne og tag screenshot
3. Sammenlign amplituden mellem de to målinger

### 10.5 Forventet Resultat

| Måling | Uden Metal | Med Metal |
|--------|------------|-----------|
| Bølgeform | Minimal/flad | Sinusformet 2kHz |
| Amplitude | ~0 mV | Øget (afhænger af metaltype) |
| Fase | - | Ændret relativt til TX |

### 10.6 Måleresultater

#### Uden Metal (Baseline)

![[../Images/Rx_before_rxamp_noMetal.png]]

**Observation:**
- Signalet er næsten fladt med minimal amplitude (~0 mV).
- RX spolen er balanceret/nullet i forhold til TX feltet.
- Dette er baseline-tilstanden hvor ingen metal forstyrrer det elektromagnetiske felt.

#### Med Stål

![[../Images/Rx_before_rxamp_Metal.png]]

**Observation:**
- Signifikant amplitudeforøgelse til ~±50-100 mV.
- Tydelig 2kHz bølgeform induceret fra TX spolen.
- Stål (ferromagnetisk metal) forstyrrer det elektromagnetiske felt og ændrer koblingen mellem TX og RX spolerne.
- De skarpe transienter skyldes H-bro switching.

### 10.7 Metaldetektionsprincip

```text
UDEN METAL:                      MED METAL:

TX ~~~→ [  ] ~~~→ RX            TX ~~~→ [METAL] ~~~→ RX
        Luft                            │
        Minimal kobling                 Forstyrret felt
        Signal ≈ 0                      Signal ↑↑↑
```

> [!info] Hvorfor Ændres Signalet?
> Ferromagnetiske metaller (stål, jern) koncentrerer det magnetiske felt og øger koblingen mellem TX og RX. Ikke-ferromagnetiske metaller (kobber, aluminium) inducerer hvirvelstrømme der skaber et modsatrettet felt. Begge effekter ændrer RX signalet.

> [!success] Test Godkendt
> RX spolen reagerer tydeligt på metal. Amplitudeforøgelsen demonstrerer at metaldetektoren kan detektere stål korrekt.

---

## 11. Fejlfinding

### 10.1 Generelle Problemer

| Problem | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Ingen PWM output | MCU kører ikke | Verificer firmware og PWM pin |
| Deadtime for lang | RC værdier forkerte | Tjek komponentværdier |
| Gate drive svag | Bootstrap ikke opladet | Sikr low-side aktivitet |
| Output oscillerer | Parasitisk induktans | Tilføj gate modstand/snubber |

### 10.2 Termiske Problemer

| Observation | Mulig Årsag | Løsning |
|-------------|-------------|---------|
| MOSFET varm | Shoot-through | Øg deadtime |
| MOSFET varm | Høj Rds(on) | Tjek gate drive niveau |
| Driver varm | Overdreven kapacitiv load | Reducer gate frekvens |

### 10.3 AD3 Specifikke Problemer

| Problem | Løsning |
|---------|---------|
| Probe loading | Brug 10:1 probe til høj-impedans punkter |
| Ground bounce | Brug kort GND forbindelse tæt på målepunkt |
| Aliasing | Øg sample rate til minimum 10× signalfrekvens |

---

## 12. Relaterede Dokumenter

- [[Testing Guide|Generel Testguide]] - AD3 opsætning og firmware test
- [[Assembly_Guide|Samlevejledning]] - Hardware opsætning
- [[Code_Review|Firmware Gennemgang]] - PWM generering
- [[../Theory/H-Bridge Theory|H-Bro Teori]] - Kredsløbsprincipper

---

*H-Bridge Måleguide til DTU 34621 Metaldetektor Projekt*
