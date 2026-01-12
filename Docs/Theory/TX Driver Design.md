# TX Driver Design - H-Bro Forstærker

> [!abstract] Dokumentformål
> Design og teori for H-bro TX driver til VLF metaldetektor.
> Konverterer 9V DC til 2kHz AC sinusstrøm gennem TX spolen.
> Simuleret effektivitet: **~98%**

---

## 1. Systemoversigt

### 1.1 Signalflow

```text
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  PWM Input  │────▶│  Dead-Time  │────▶│  Bootstrap  │────▶│   H-Bridge  │────▶ TX Spole
│   (2kHz)    │     │  Generator  │     │ Gate Driver │     │  (IRL530)   │     (LC Tank)
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

### 1.2 Designkrav

| Parameter | Værdi | Kilde |
|-----------|-------|-------|
| TX Frekvens | 2 kHz | Firmware |
| TX Strøm | ~80 mA | Strømbudget |
| Forsyning | 9V (batteri) | Hardware |
| Effektivitet | > 90% | Kravspecifikation |
| Dead-time | ~15 µs | Shoot-through forebyggelse |

---

## 2. H-Bro Topologi

### 2.1 Hvorfor H-Bro?

> [!tip] DC til AC Konvertering
> H-broen skifter lastkoblingen mellem +V og GND med 180° faseforskydning.
> Kombineret med LC resonanskredsen producerer dette en ren sinusbølge.

**Fordele ved full-bridge (H-bro):**
- Dobbelt spændingssving sammenlignet med half-bridge
- Ingen DC offset i lasten
- Symmetrisk strømflow

### 2.2 Kredsløbsdiagram

```text
        +9V (V1, RSER=1.7Ω)
            │
    ┌───────┴───────┐
    │               │
   M6              M3
  (IRL530)       (IRL530)      ← High-side switches
    │               │
    ├───[L2]───[C1]─┤───[R10]───┐
    │   6.3mH   1µF │    35Ω    │   ← LC Tank (TX spole)
   M4              M2           │
  (IRL530)       (IRL530)       │   ← Low-side switches
    │               │           │
    └───────┬───────┘           │
           GND◄─────────────────┘
```

### 2.3 Switching Sekvens

| Fase | M6 (High-L) | M4 (Low-L) | M3 (High-R) | M2 (Low-R) | Laststrøm |
|------|-------------|------------|-------------|------------|-----------|
| A | ON | OFF | OFF | ON | +9V → Load → GND |
| Dead | OFF | OFF | OFF | OFF | Freewheel |
| B | OFF | ON | ON | OFF | GND ← Load ← +9V |
| Dead | OFF | OFF | OFF | OFF | Freewheel |

### 2.4 Komponentspecifikationer

| Komponent | Part | Specifikationer |
|-----------|------|-----------------|
| M2, M3, M4, M6 | IRL530 | Vds=100V, Ids=15A, Ron=0.16Ω, Vgs(th)=2V |
| D3, D4 | 1N4148 | Freewheeling dioder |

> [!note] Logic-Level MOSFETs
> IRL530 er en logic-level MOSFET med Vgs(th)=2V.
> Dette tillader direkte drivning fra 5V MCU signaler.

---

## 3. Bootstrap Gate Driver

### 3.1 Problemet

> [!warning] High-Side N-kanal Udfordring
> For at tænde en N-kanal MOSFET skal gate være **højere** end source.
> For high-side switch er source = load voltage, som kan være op til +9V.
> Gate skal derfor være +9V **over** source = +18V totalt!

### 3.2 Løsningen: Bootstrap

Bootstrap kredsløbet skaber en **flydende forsyning** over high-side MOSFET'en:

```text
    +9V
     │
     D1 (BAS521)──────┐
     │                │
    [R1=5Ω]          [C1=1µF]  ← Bootstrap kondensator
     │                │
     └────────────────┼──── Vboot (flydende forsyning)
                      │
                     M1 (High-side)
                      │
                     OUT ──────────────────── Til last
                      │
                     M2 (Low-side)
                      │
                     GND
```

### 3.3 Bootstrap Sekvens

1. **Low-side ON**: M2 leder, OUT = GND
2. **C1 oplader**: Strøm flyder +9V → D1 → R1 → C1 → GND
3. **Low-side OFF**: Dead-time begynder
4. **High-side ON**: M1 gate drives af Vboot (OUT + 9V)
5. **C1 aflader**: Leverer gate-ladning til M1

> [!tip] Bootstrap Kondensator Størrelse
> C1 skal være stor nok til at levere gate-ladning Qg uden signifikant spændingsfald:
> $$\Delta V = \frac{Q_g}{C} = \frac{17.8\text{nC}}{1\mu\text{F}} = 17.8\text{mV}$$
> Med 1µF er spændingsfaldet negligerbart.

### 3.4 Hvorfor N-kanal på Begge Sider?

| Parameter | N-kanal | P-kanal |
|-----------|---------|---------|
| Rds(on) for samme die | **Lavere** | Højere |
| Effektivitet | **Højere** | Lavere |
| Gate drive | Kræver bootstrap | Simpel |

> [!note] Trade-off
> Bootstrap tilføjer kompleksitet, men giver **højere effektivitet**
> fordi N-kanal MOSFETs har lavere Rds(on).

---

## 4. Dead-Time Generator

### 4.1 Shoot-Through Problemet

> [!danger] Shoot-Through
> Hvis high-side og low-side i samme ben tænder samtidigt,
> opstår en kortslutning fra +9V til GND.
> Dette kan **ødelægge MOSFETs** og **dræne batteriet**.

Dead-time sikrer at begge switches er OFF under skift.

### 4.2 RC Delay Løsning

```text
                    ┌─────────────┐
     PWM_In ───────▶│   RC Delay  │
                    │  R4=R, C1=C │
                    └──────┬──────┘
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
        ┌──────────┐              ┌──────────┐
        │  OpAmp1  │              │  OpAmp2  │
        │ (comp H) │              │ (comp L) │
        └────┬─────┘              └────┬─────┘
             │                         │
             ▼                         ▼
          H output                  L output
       (high-side)               (low-side)
```

### 4.3 Timing Netværk

| Komponent | Værdi | Funktion |
|-----------|-------|----------|
| R1 | 1850Ω | Spændingsdeler |
| R2 | 650Ω | Spændingsdeler |
| R3 | 2.5kΩ | Tærskel indstilling |
| R4 | R (param) | Dead-time justering |
| C1 | C (param) | Dead-time justering |

### 4.4 Dead-Time Beregning

$$t_{dead} \approx R \cdot C$$

Med R=3kΩ, C=5nF:
$$t_{dead} \approx 3\text{k}\Omega \cdot 5\text{nF} = 15\mu\text{s}$$

> [!note] Timing Sekvens
> - PWM stiger: L slukker først, derefter H tænder (efter dead-time)
> - PWM falder: H slukker først, derefter L tænder (efter dead-time)

---

## 5. LC Resonanskreds

### 5.1 Firkant til Sinus Konvertering

```text
  MCU Firkantbølge              LC Tank Output
  ┌───┐   ┌───┐                  ╭───╮   ╭───╮
  │   │   │   │                 ╱     ╲ ╱     ╲
──┘   └───┘   └──      →    ──╱       ╳       ╲──
                              ╲     ╱ ╲     ╱
                               ╰───╯   ╰───╯
```

> [!tip] LC Tank Funktion
> LC kredsen fungerer som et **båndpasfilter** der kun passerer grundfrekvensen (2kHz).
> Harmoniske fra firkantbølgen dæmpes kraftigt.

### 5.2 Resonansfrekvens

$$f_0 = \frac{1}{2\pi\sqrt{LC}} = \frac{1}{2\pi\sqrt{6.332\text{mH} \cdot 1\mu\text{F}}} \approx 2\text{kHz}$$

### 5.3 Komponentværdier

| Komponent | Værdi | Formål |
|-----------|-------|--------|
| L2 | 6.332 mH | TX spole induktans |
| C1 | 1 µF | Resonanskondensator |
| R10 | 35Ω | Spole DC modstand |

### 5.4 Q-Faktor

$$Q = \frac{X_L}{R} = \frac{2\pi f L}{R} = \frac{2\pi \times 2000 \times 0.00633}{35} = 2.27$$

> [!note] Moderat Q
> Q ≈ 2.3 giver god sinusform uden for skarp resonans.
> Højere Q ville give renere sinus men smallere båndbredde.

---

## 6. Simuleringsresultater

### 6.1 Effektivitet

| Parameter | Værdi |
|-----------|-------|
| Source Power (Psource) | ~500 mW |
| Load Power (Pl) | ~490 mW |
| Effektivitet (η) | **~98%** |

### 6.2 QSPICE Simulation

Simulationsfiler findes i `QSPICE/` mappen:

| Fil | Beskrivelse |
|-----|-------------|
| `Hbro_sim.qsch` | Hovedsimulation med H-bro og last |
| `bootstrap.qsch` | Bootstrap gate driver kredsløb |
| `dead_time.qsch` | Dead-time generator subcircuit |

Se [[../../QSPICE/README|QSPICE README]] for detaljer om simulering.

### 6.3 Vigtige Bølgeformer

```spice
.plot i(L2)              ; Spolestrøm (bør være sinusformet)
.plot is(M6) is(M3)      ; MOSFET strømme
.plot v(G9) v(G3,S3)     ; Gate-source spændinger
```

---

## 7. Stykliste

### 7.1 Power Stage

| Komponent | Part | Antal | Noter |
|-----------|------|-------|-------|
| MOSFET | IRL530 | 4 | Logic-level, TO-220 |
| Diode | 1N4148 | 2 | Freewheeling |

### 7.2 Gate Driver

| Komponent | Part | Antal | Noter |
|-----------|------|-------|-------|
| MOSFET | BS170 | 2 | Level shifter |
| Diode | 1N4148 | 2 | Bootstrap |
| Kondensator | 1µF | 2 | Bootstrap |
| Modstand | 1K | 2 | Pull-up |

### 7.3 Dead-Time

| Komponent | Værdi | Antal | Noter |
|-----------|-------|-------|-------|
| Modstand | 3K | 1 | Parameteriserbar |
| Kondensator | 5nF | 1 | Parameteriserbar |
| Op-amp | RRopAmp | 2 | Rail-to-rail |

### 7.4 LC Tank

| Komponent | Værdi | Antal | Noter |
|-----------|-------|-------|-------|
| Spole | 6.33 mH | 1 | TX spole |
| Kondensator | 1µF, 50V | 1 | Film type (MKP) |

---

## 8. Relaterede Dokumenter

- [[Coil Design|Spole Design]] - TX spole specifikationer
- [[Power Budget Analysis|Strømbudget Analyse]] - Systemets strømforbrug
- [[DFT Algorithm|DFT Algoritme]] - Signalbehandling af RX signal
- [[../../QSPICE/README|QSPICE Simulation]] - Detaljeret simulationsguide

---

## 9. Teori Referencer

| Emne | Relevans |
|------|----------|
| H-bridge switching | DC-AC konvertering |
| Bootstrap gate drive | High-side N-kanal drivning |
| LC resonance | Båndpasfiltrering |
| MOSFET switching losses | Effektivitetsberegning |

---

*TX Driver design til DTU 34621 Metaldetektor Projekt*
*Simuleret i QSPICE med ~98% effektivitet*
