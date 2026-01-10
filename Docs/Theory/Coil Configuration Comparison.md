# Spole Konfiguration: Koncentrisk vs Double-D

> [!abstract] Dokumentformål
> Sammenligning af spolekonfigurationer til VLF metaldetektor design.
> Begrundelse for valg af koncentrisk konfiguration.

---

## 1. Hurtig Sammenligning

| Aspekt | Koncentrisk | Double-D (DD) |
|--------|-------------|---------------|
| **Konstruktion** | Nemmere (cirkulære spoler) | Sværere (D-formet, præcis overlap) |
| **Følsomhed** | Bedre (ikke-mineraliseret jord) | Lidt mindre |
| **Jordafvisning** | Dårlig i mineraliseret jord | Fremragende |
| **Pinpointing** | Bedst (center-fokuseret) | God (klinge-mønster) |
| **Diskriminering** | Bedre | God |
| **Dækning per sweep** | Kegle-mønster | Bredere klinge-mønster |
| **Dybde** | God | Bedre i dårlig jord |
| **Detektionsmønster** | Kegle (cirkulær) | Klinge (linje) |

---

## 2. Detektionsfelt Mønstre

### 2.1 Koncentrisk

```
        Set Ovenfra              Tværsnit (Side)
      ┌─────────┐                    ╱╲
     ╱    RX    ╲                   ╱  ╲
    │   ┌───┐    │                 ╱    ╲
    │   │ B │    │  TX            ╱ KEGLE ╲
    │   └───┘    │               ╱        ╲
     ╲          ╱               ▼──────────▼
      └─────────┘              Bedst detektion i center
```

- **Mønster:** Kegleformet, stærkest i centrum
- **Pinpointing:** Fremragende - mål er direkte under center
- **Sweep:** Kræver mere overlap mellem sweeps

### 2.2 Double-D

```
        Set Ovenfra              Tværsnit (Side)
      ┌─────────┐
     ╱ D     D  ╲                 │        │
    │   ╲   ╱    │               ╱│        │╲
    │    ╲ ╱     │              ╱ │ KLINGE │ ╲
    │    ╱ ╲     │             ╱  │        │  ╲
     ╲  ╱   ╲   ╱             ▼───┴────────┴───▼
      └─────────┘              Detektion langs centerlinje
         ▲
    Overlap zone
   (detektionsområde)
```

- **Mønster:** Klingeformet langs overlap-linjen
- **Pinpointing:** God - mål er under centerlinjen
- **Sweep:** Bedre jorddækning per passage

---

## 3. Hardware Sammenligning

### 3.1 Koncentrisk Konfiguration (Vores Design)

```
┌──────────────────────────────────────┐
│            TX Spole (ydre)           │  200 mm - Sender
│   ┌────────────────────────────┐     │
│   │      Bucking Spole         │     │  120 mm - Ophævning
│   │   ┌────────────────────┐   │     │
│   │   │     RX Spole       │   │     │  80 mm - Modtager
│   │   │                    │   │     │
│   │   └────────────────────┘   │     │
│   └────────────────────────────┘     │
└──────────────────────────────────────┘
```

**Komponenter:**

| Komponent | Formål | Specifikationer |
|-----------|--------|-----------------|
| TX Spole | Sender 2 kHz felt | 200 mm, 68 vind., 6.33 mH |
| Bucking Spole | Ophæver TX kobling til RX | 120 mm, ~35 vind. |
| RX Spole | Modtager reflekteret signal | 80 mm, ~170 vind., ≥10 mH |

**Balancering:** Bucking spolen ophæver den direkte TX→RX kobling, så kun metalrefleksionen detekteres.

### 3.2 Double-D Konfiguration

```
┌─────────────────────────────────────┐
│                                     │
│    ┌─────────┐  ┌─────────┐        │
│    │         │  │         │        │
│    │   TX    │╲╱│   RX    │        │
│    │    D    │╱╲│    D    │        │
│    │         │  │         │        │
│    └─────────┘  └─────────┘        │
│                                     │
└─────────────────────────────────────┘
        ▲           ▲
        └─────┬─────┘
         Overlap zone
      (induktionsbalanceret)
```

**Komponenter:**

| Komponent | Formål | Noter |
|-----------|--------|-------|
| TX Spole (D-formet) | Sender felt | Halvdel |
| RX Spole (D-formet) | Modtager signal | Anden halvdel, overlapper TX |

**Balancering:** Overlap-geometrien skaber naturligt en nulzone hvor TX kobling ophæves.

---

## 4. Elektriske Parametre

### 4.1 Vores Koncentriske Design (Arduino Nano)

| Parameter | TX Spole | RX Spole | Bucking |
|-----------|----------|----------|---------|
| Diameter | 200 mm | 80 mm | 120 mm |
| Induktans | 6.33 mH | ~12 mH | ~3 mH |
| DC Modstand | ~3.5 Ω | ~9 Ω | ~1 Ω |
| Vindinger | 68 | ~170 | ~35 |
| Resonans C | 1.0 µF | 470 nF | - |

### 4.2 Ækvivalent Kredsløb (Begge typer)

```
TX Side                    RX Side
   │                          │
  ┌┴┐ R_tx                   ┌┴┐ R_rx
  └┬┘                        └┬┘
   │                          │
  ┌┴┐                        ┌┴┐
  │ │ L_tx                   │ │ L_rx
  └┬┘                        └┬┘
   │                          │
   └──────── M (gensidig) ────┘
```

---

## 5. Software/DSP Forskelle

### 5.1 Hvad Forbliver Ens

| Komponent | Noter |
|-----------|-------|
| DFT beregning | Samme single-bin ved 2 kHz |
| IIR filtrering | Samme udjævningsalgoritme |
| Fasedetektion | Samme ferro/ikke-ferro tærskel |
| Samplingsrate | Samme 8 kHz |
| Timer konfiguration | Samme hardware PWM |

### 5.2 Hvad Kan Kræve Justering

| Parameter | Koncentrisk | Double-D | Kode Påvirkning |
|-----------|-------------|----------|-----------------|
| Signal amplitude | Højere | Lavere | Juster normalisering |
| Kalibreringsværdier | Forskellige | Forskellige | Genkør kalibrering |
| Detektionstærskel | Kan variere | Kan variere | Juster tærskel |
| Faseoffset | Kan variere | Kan variere | Juster kalibrering |
| Forstærker gain | Mindre behov | Mere behov | Hardware ændring |

**Konklusion:** Spolerne er stort set udskiftelige i software - kun kalibreringskonstanter ændres.

---

## 6. Konstruktionskompleksitet

### 6.1 Koncentrisk

**Sværhedsgrad:** Medium

1. Vikl cirkulær TX spole (nemt)
2. Vikl cirkulær bucking spole (nemt)
3. Vikl cirkulær RX spole (nemt)
4. **Kritisk:** Positioner bucking spole for nulbalance
5. Finjuster med enkelt trådløkke
6. Støb i epoxy

**Udfordringer:**
- Opnå præcis bucking spole position
- Oprethold balance efter støbning

### 6.2 Double-D

**Sværhedsgrad:** Svær

1. Lav D-formede forme (kræver 3D print eller laserskæring)
2. Vikl D-formet TX spole (sværere at vikle jævnt)
3. Vikl D-formet RX spole (sværere at vikle jævnt)
4. **Kritisk:** Præcis overlap-positionering for nul
5. Støb i epoxy

**Udfordringer:**
- D-formet vikling er vanskelig
- Overlap-geometri skal være præcis
- Mekaniske tolerancer er stramme

---

## 7. Jordbalanceringsydelse

### 7.1 Hvorfor DD er Bedre i Mineraliseret Jord

```
Koncentrisk i mineraliseret jord:
┌────────────────────────────────┐
│  TX felt komprimeres           │
│  ujævnt af jordmineraler       │
│                                │
│    ╱──────╲   Trykket          │
│   ╱   RX   ╲  felt             │
│  │  ┌───┐   │ skaber           │
│   ╲ └───┘  ╱  ubalance         │
│    ╲──────╱                    │
└────────────────────────────────┘
Resultat: Falske signaler fra jord

Double-D i mineraliseret jord:
┌────────────────────────────────┐
│  TX komprimering påvirker      │
│  begge halvdele ens            │
│                                │
│   ┌───┐╲╱┌───┐                │
│   │ T │╱╲│ R │ Balance        │
│   └───┘  └───┘ opretholdes    │
│                                │
└────────────────────────────────┘
Resultat: Jordeffekt ophæves
```

---

## 8. Anbefaling for Projektet

### 8.1 Vælg Koncentrisk Hvis:

- Første gang med spolebyggeri
- Test i laboratorium (lav mineralisering)
- Ønsker nemmere konstruktion
- Behov for bedste pinpointing nøjagtighed
- Begrænset tid

### 8.2 Vælg Double-D Hvis:

- Har adgang til 3D printer til D-forme
- Ønsker bedste ydelse i felten
- Planlægger at teste udendørs (moderat mineralisering)
- Teamet har erfaring med spolevinding
- Ønsker læringsoplevelsen

### 8.3 Vores Valg: Koncentrisk

> [!success] Valgt Konfiguration
> **Koncentrisk** konfiguration er valgt for dette projekt:
> - Nemmere at konstruere inden for tidsrammen
> - Bedre pinpointing til demonstrationsformål
> - Samme PCB og kode virker - kun kalibrering ændres
> - Mulighed for DD som fremtidig opgradering

---

## 9. Stykliste Sammenligning

### 9.1 Koncentrisk (Vores Design)

| Del | Antal | Noter |
|-----|-------|-------|
| Kobbertråd 0.52mm (AWG 24) | ~60 m | TX + Bucking |
| Kobbertråd 0.32mm (AWG 28) | ~45 m | RX |
| Cirkulær form 200 mm | 1 | TX spole |
| Cirkulær form 120 mm | 1 | Bucking spole |
| Cirkulær form 80 mm | 1 | RX spole |
| Epoxy/støbemasse | ~200 ml | |
| Film kondensator 1.0 µF | 1 | TX resonans |
| Film kondensator 470 nF | 1 | RX resonans (valgfri) |

### 9.2 Double-D (Reference)

| Del | Antal | Noter |
|-----|-------|-------|
| Kobbertråd | ~40 m | Begge D-spoler |
| D-formet form | 2 | 3D printet, ~200 mm |
| Overlap jig | 1 | Holder D'er i position |
| Epoxy/støbemasse | ~200 ml | |

---

## 10. Relaterede Dokumenter

- [[Coil Design|Spole Design]] - Detaljeret spoledesign
- [[TX Driver Design|TX Driver Design]] - Forstærker kredsløb
- [[Power Budget Analysis|Strømbudget Analyse]] - Strømforbrug

---

## 11. Teori Referencer (DTU Vault)

| Emne | Link | Relevans |
|------|------|----------|
| Magnetfelter | [Lecture 22 - Magnetostatics I](obsidian://open?vault=Obsidian&file=Courses%2FElectromagnetics%2FFormulas%2FLecture%2022%20-%20Magnetostatics%20I) | Feltmønstre omkring spoler |
| Induktans | [L23 - Magnetostatics II](obsidian://open?vault=Obsidian&file=Courses%2FElectromagnetics%2FFormulas%2FL23%20-%20Magnetostatics%20II) | Gensidig induktans, kobling |
| Skin Effect | [Helpers (EM)](obsidian://open?vault=Obsidian&file=Courses%2FElectromagnetics%2FHelpers) | Hvirvelstrømme, metalklassificering |

---

*Sammenligning af spolekonfigurationer til DTU 34621 Metaldetektor*

#spoler #design #koncentrisk #double-d
