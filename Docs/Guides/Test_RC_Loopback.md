# RC Loopback Test til Verifikation af Fasemåling

## 1. Formål

Denne test verificerer DFT-faseberegningen ved at bruge et simpelt RC lavpasfilter til at skabe et kendt, beregnelig faseskift mellem TX og RX. Ved at forbinde Arduinoens TX-udgang gennem passive komponenter direkte til ADC-indgangen eliminerer vi synkroniseringsproblemer og kan verificere at fasemålingen ændrer sig forudsigeligt med forskellige RC-værdier.

## 2. Teori

### Basisfase (-45°)

TX-signalet er en **firkantbølge**, ikke en sinusbølge. Når det måles uden RC-filtrering (kun modstand, ingen kondensator), rapporterer DFT'en en basisfase på **-45°**.

Dette sker fordi en firkantbølge der er HØJ for samples 0,1 og LAV for samples 2,3 producerer:

```
Re = x[0] - x[2] = (+A) - (-A) = +2A
Im = -x[1] + x[3] = (-A) + (-A) = -2A
Fase = atan2(-2A, +2A) = -45°
```

### RC Filter Faseskift

Et RC lavpasfilter introducerer yderligere faseskift. For en ren sinusbølge:

```
Faseskift = -arctan(2π × f × R × C)
```

Dog, fordi vi filtrerer en firkantbølge (ikke en sinus), er det faktiske faseskift cirka **dobbelt** så stort som den teoretiske sinusbølge-værdi på grund af bølgeform-omformningseffekter.

**Eksempel med 1kΩ og 100nF ved 2 kHz:**

```
Teoretisk (sinus):    -arctan(2π × 2000 × 1000 × 100×10⁻⁹) ≈ -52°
Faktisk (firkantbølge): cirka -100°
```

### Total Målt Fase

```
Total Fase = Basisfase (-45°) + RC Filter Skift
```

Med 1kΩ + 100nF: **-45° + (-100°) = -145°**

## 3. Nødvendige Komponenter

| Komponent | Værdi | Antal | Noter |
|-----------|-------|-------|-------|
| Modstand | 1 kΩ | 1 | 1/4W eller lignende |
| Kondensator | 100 nF | 1 | Keramisk eller film |
| Jumperwires | - | 2 | Til forbindelser |

## 4. Kredsløbsdiagram

### Skematisk

```
                    R (1kΩ)
    D9 (TX) ───────/\/\/\/───────┬─────── A0 (RX)
                                 │
                                 │
                              ───┴───
                              ───┬─── C (100nF)
                                 │
                                 │
    GND ─────────────────────────┴─────── GND
```

### Breadboard Beskrivelse

1. Forbind en jumperwire fra **D9** til det ene ben af **1kΩ modstanden**
2. Forbind det andet ben af modstanden til:
   - **A0** (jumperwire)
   - Det ene ben af **100nF kondensatoren**
3. Forbind det andet ben af kondensatoren til **GND**

### Forbindelsesoversigt

| Arduino Pin | Forbindes Til |
|-------------|---------------|
| D9 (TX) | Modstand indgang |
| A0 (RX) | Modstand udgang + Kondensator |
| GND | Kondensator stel |

## 5. Firmware Konfiguration

Brug standard `main.c` firmware uden ændringer. Testen bruger normal intern TX-tilstand hvor Arduinoen genererer 2 kHz firkantbølgen på D9 og sampler det filtrerede signal på A0.

## 6. Forventede Resultater

### Uden Kondensator (Basistest)

| Måling | Forventet Værdi | Tolerance |
|--------|-----------------|-----------|
| Fase | -45° | ±5° |
| Magnitude | Stabil, ikke-nul | - |

### Med 1kΩ + 100nF

| Måling | Forventet Værdi | Tolerance |
|--------|-----------------|-----------|
| Fase | -145° | ±15° |
| Magnitude | Stabil, ikke-nul | >10 |

**Bemærk:** Den viste fase bør være stabil og ikke hoppe mellem værdier. Små variationer på ±3° er normale på grund af komponenttolerancer og kvantisering.

## 7. Alternative Komponentværdier

Forskellige R/C kombinationer producerer forskellige faseskift. Basisfasen på -45° er altid til stede.

| Modstand | Kondensator | Målt Fase | Skift fra Basis |
|----------|-------------|-----------|-----------------|
| Enhver | Ingen | -45° | 0° (basis) |
| 560 Ω | 100 nF | ~ -100° | ~ -55° |
| 1 kΩ | 100 nF | ~ -145° | ~ -100° |
| 2.2 kΩ | 100 nF | ~ -160° | ~ -115° |
| 4.7 kΩ | 100 nF | ~ -170° | ~ -125° |

**Bemærk:** Eksakte værdier afhænger af komponenttolerancer. Det vigtige er at øgning af R eller C bør gøre fasen mere negativ.

## 8. Fejlfinding

| Symptom | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Fase nær -45° med kondensator | Kondensator ikke forbundet | Verificer kondensator er forbundet mellem A0-node og GND |
| Fase nær -45° med kondensator | Kondensator kortsluttet eller defekt | Udskift kondensator |
| Fase hopper | Løse forbindelser | Sørg for alle forbindelser er sikre, brug kortere ledninger |
| Fase positiv | Uventet - tjek ledningsføring | Verificer D9→R→A0 og C→GND forbindelser |
| Magnitude meget lav (<5) | Forkert modstandsværdi | Verificer modstand er 1kΩ, ikke 1MΩ |
| Magnitude meget lav | D9 sender ikke | Tjek TX signal med oscilloskop eller LED |
| Magnitude nul | A0 ikke forbundet | Verificer A0 forbindelse til RC-knudepunkt |
| Ustabile aflæsninger | Strømforsyningsstøj | Tilføj afkoblingskondensator nær Arduino |

## 9. Hvorfor Denne Test Virker

Denne test er pålidelig fordi:

1. **Iboende synkronisering**: TX og RX stammer fra samme kilde. RX-signalet er simpelthen TX-signalet sendt gennem passive komponenter - ingen separate clock-domæner eller synkroniseringsproblemer.

2. **Forudsigelig opførsel**: Selvom de eksakte faseværdier afhænger af firkantbølge-effekter, skifter fasen **forudsigeligt** med forskellige RC-værdier.

3. **Intet eksternt udstyr nødvendigt**: I modsætning til tests der kræver signalgeneratorer, kræver denne kun basale passive komponenter.

4. **Verificerer hele signalkæden**: Tester ADC-indgang, DFT-beregning, faseberegning og display-output.

## 10. Vigtige Noter

- Basisfasen på -45° er **normal** og forventet på grund af firkantbølge-sampling
- For metaldetektion er kun **ændringer** i fase vigtige, ikke absolutte værdier
- Basisfasen udlignes når målinger sammenlignes
- Faseværdier nær ±180° kan wrappe (f.eks. -175° kan vises som +185° eller omvendt)

---

## Godkendelseskriterier

- [ ] Uden kondensator: Fase aflæser cirka -45° (±5°)
- [ ] Med 1kΩ + 100nF: Fase aflæser cirka -145° (±15°)
- [ ] Faseaflæsning er stabil (hopper ikke)
- [ ] Magnitude er ikke-nul og stabil
- [ ] Øgning af R-værdi gør fasen mere negativ
- [ ] Fjernelse af kondensator returnerer fasen til basis (-45°)

**Teststatus:** _____________ **Dato:** _____________ **Tester:** _____________
