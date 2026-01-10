# TX Driver Design

> [!abstract] Dokumentformål
> Dokumentation af H-bro TX driver kredsløb til metaldetektor sendespole.
> Design simuleret i LTspice.

---

## 1. Kredsløbsoversigt

### 1.1 Topologi

**Full H-bro** med komplementære MOSFET par:
- 2× P-kanal high-side switches (IRF5305PbF)
- 4× N-kanal MOSFETs (IRL530) - low-side switches og gate drivere

### 1.2 Skematisk Diagram

```
                              V5 (+9V)
                                 │
              ┌──────────────────┼──────────────────┐
              │                  │                  │
           ┌──┴──┐            ┌──┴──┐            ┌──┴──┐
           │ R3  │            │     │            │ R2  │
           │ 5K  │            │     │            │ 5K  │
           └──┬──┘            │     │            └──┬──┘
              │               │     │               │
         ┌────┴────┐     ┌────┴─────┴────┐     ┌────┴────┐
         │   M5    │     │  M2      M1   │     │   M6    │
    V1 ──┤ IRL530  ├─────┤ IRF5305  IRF5305├─────┤ IRL530  ├── V2
         │ (N-ch)  │     │ (P-ch)  (P-ch)│     │ (N-ch)  │
         └────┬────┘     └────┬─────┬────┘     └────┬────┘
              │               │     │               │
              ▼ GND           │     │               ▼ GND
                              │     │
                         ┌────┴─────┴────┐
                         │               │
                       ──┴──           ──┴──
                       ──┬── C1        ──┬──
                         │   10µF        │
                         │               │
                        ╔╧╗             ╱╲
                        ║L1║           ╱  ╲ R1
                        ║6.33mH       ╲  ╱ 35Ω
                        ╚╤╝             ╲╱
                         │               │
                         └───────┬───────┘
                                 │
                         ┌───────┴───────┐
                         │               │
                    ┌────┴────┐     ┌────┴────┐
                    │   M4    │     │   M3    │
               V4 ──┤ IRL530  │     │ IRL530  ├── V3
                    │ (N-ch)  │     │ (N-ch)  │
                    └────┬────┘     └────┬────┘
                         │               │
                         ▼ GND           ▼ GND
```

---

## 2. Komponentliste

### 2.1 MOSFETs

| Ref | Komponent | Type | Nøgle Specifikationer |
|-----|-----------|------|----------------------|
| M1, M2 | IRF5305PbF | P-kanal | Vds=-55V, Id=-31A, Rds(on)=0.06Ω |
| M3, M4, M5, M6 | IRL530 | N-kanal (Logic Level) | Vds=100V, Id=15A, Rds(on)=0.16Ω |

### 2.2 Passive Komponenter

| Ref | Værdi | Funktion |
|-----|-------|----------|
| L1 | 6.33 mH | TX sendespole |
| R1 | 35 Ω | Spole DC modstand + serie modstand |
| C1 | 10 µF | DC blokering / resonans |
| R2, R3 | 5 kΩ | Gate pull-up modstande |

### 2.3 Forsyning

| Ref | Værdi | Noter |
|-----|-------|-------|
| V5 | 9V DC | Batterforsyning |

---

## 3. Driftsprincip

### 3.1 H-bro Switching

H-broen drives med komplementære signaler for at skabe vekselstrøm gennem spolen:

| Fase | Venstre Side | Højre Side | Spolestrøm |
|------|--------------|------------|------------|
| A | M2 ON, M4 ON | M1 OFF, M3 OFF | → (positiv) |
| B | M2 OFF, M4 OFF | M1 ON, M3 ON | ← (negativ) |

### 3.2 Gate Driver Kredsløb

P-kanal MOSFETs (M1, M2) kræver gate spænding UNDER source for at tænde.
N-kanal MOSFETs (M5, M6) bruges som gate drivere:

```
Når V1 = HIGH (5V):
  M5 tænder → trækker M2 gate til GND
  M2 gate = 0V, M2 source = 9V
  Vgs = 0 - 9 = -9V → M2 tænder

Når V1 = LOW (0V):
  M5 slukker → R3 trækker M2 gate til 9V
  Vgs = 9 - 9 = 0V → M2 slukker
```

### 3.3 Timing Diagram

```
         ◄──────── 500 µs (2 kHz) ────────►

V1/V3:   ┌────────────────────────┐        ┌────
    5V ──┤                        │        │
    0V   └────────────────────────┴────────┘
         │◄────── 240 µs ────────►│◄─260µs─►│

V2/V4:        ┌────────────────────────┐
    5V ───────┤                        │────────
    0V   ─────┘                        └────────

Spole:   ┌────────────────────────┐
   +9V ──┤      M2+M4 ON          │
    0V   │                        │
   -9V   └────────────────────────┴── M1+M3 ON
```

---

## 4. Simuleringsparametre

### 4.1 LTspice Pulskilder

| Kilde | PULSE Parametre | Beskrivelse |
|-------|-----------------|-------------|
| V1 | (0 5 0 1µ 1µ 240µ 500µ) | Venstre high-side driver |
| V2 | (5 0 0 1µ 1µ 240µ 500µ) | Højre high-side driver (inverteret) |
| V3 | (0 5 0 1µ 1µ 240µ 500µ) | Højre low-side |
| V4 | (5 0 0 1µ 1µ 240µ 500µ) | Venstre low-side (inverteret) |

**PULSE format:** (V1 V2 Tdelay Trise Tfall Ton Tperiod)

### 4.2 Timing Beregninger

| Parameter | Værdi | Beregning |
|-----------|-------|-----------|
| Periode | 500 µs | 1/2 kHz |
| Pulsbredde | 240 µs | ~48% duty cycle |
| Rise/Fall tid | 1 µs | Hurtig switching |
| Frekvens | 2 kHz | TX frekvens |

### 4.3 Transient Analyse

```spice
.tran 0 1 0.998 1µ
```
- Simulerer 1 sekund
- Start måling ved 0.998s (steady-state)
- Tidstrin: 1 µs

---

## 5. Elektriske Beregninger

### 5.1 Spoleimpedans

Ved 2 kHz:

$$X_L = 2\pi f L = 2\pi \times 2000 \times 0.00633 = 79.5\ \Omega$$

$$Z_{total} = \sqrt{R^2 + X_L^2} = \sqrt{35^2 + 79.5^2} = 86.9\ \Omega$$

### 5.2 Spolestrøm

Med H-bro (±9V swing = 18V peak-to-peak):

$$I_{peak} = \frac{V_{pp}/2}{Z} = \frac{9}{86.9} = 103\ \text{mA}$$

$$I_{rms} = \frac{I_{peak}}{\sqrt{2}} = 73\ \text{mA}$$

### 5.3 Effektberegninger

**Effekt i spolemodstand:**
$$P_{R1} = I_{rms}^2 \times R = 0.073^2 \times 35 = 186\ \text{mW}$$

**Total effekt fra forsyning:**
$$P_{total} \approx V \times I_{avg} = 9V \times 80mA = 720\ \text{mW}$$

### 5.4 MOSFET Tab

**Konduktions tab (per MOSFET par):**

High-side (IRF5305):
$$P_{cond,HS} = I_{rms}^2 \times R_{ds(on)} = 0.073^2 \times 0.06 = 0.32\ \text{mW}$$

Low-side (IRL530):
$$P_{cond,LS} = I_{rms}^2 \times R_{ds(on)} = 0.073^2 \times 0.16 = 0.85\ \text{mW}$$

**Total MOSFET tab: < 5 mW** (negligibelt)

---

## 6. Resonansovervejelser

### 6.1 LC Tank Kredsløb

Med C1 = 10 µF og L1 = 6.33 mH:

$$f_{resonans} = \frac{1}{2\pi\sqrt{LC}} = \frac{1}{2\pi\sqrt{0.00633 \times 0.00001}} = 632\ \text{Hz}$$

> [!warning] Ikke Resonant ved 2 kHz
> Kredsløbet er IKKE i resonans ved 2 kHz. Kondensatoren fungerer primært som DC-blokering.

---

> [!danger] UDVIKLER NOTE: Ændring Påkrævet for Resonans
>
> **Nuværende:** C1 = 10 µF → f_resonans = 632 Hz (FORKERT)
>
> **Påkrævet:** C1 = **1.0 µF** → f_resonans = 2 kHz (KORREKT)
>
> ```
> Beregning:
> C = 1 / ((2π × f)² × L)
> C = 1 / ((2π × 2000)² × 0.00633)
> C = 1.0 µF
> ```
>
> **Anbefalede komponenter:**
> - 1.0 µF film kondensator (MKP/MKT)
> - Spændingsrating: minimum 50V (Q-faktor kan forstærke spænding!)
> - Lav ESR type for bedre Q
>
> **Fordele ved resonans:**
> - Højere spolestrøm ved samme forsyningsstrøm
> - Bedre energieffektivitet
> - Stærkere magnetfelt

---

### 6.2 Resonant Tank Beregninger

For resonans ved 2 kHz med L = 6.33 mH:

$$C = \frac{1}{(2\pi f)^2 L} = \frac{1}{(2\pi \times 2000)^2 \times 0.00633} = 1.0\ \text{µF}$$

**Q-faktor:**
$$Q = \frac{X_L}{R} = \frac{79.5}{35} = 2.3$$

**Spændingsforstærkning ved resonans:**

Ved resonans vil spændingen over spolen være Q gange højere end input:
$$V_{L,peak} = Q \times V_{in} = 2.3 \times 9V = 20.7V$$

> [!warning] Højspænding ved Resonans
> Ved resonans kan spændingen over L og C overstige forsyningsspændingen!
> Sørg for at C1 har tilstrækkelig spændingsrating (≥50V anbefales).

---

## 7. Komponent Valg Begrundelse

### 7.1 IRF5305PbF (P-kanal High-Side)

| Parameter | Værdi | Krav |
|-----------|-------|------|
| Vds | -55V | > 9V ✓ |
| Id | -31A | > 0.1A ✓ |
| Rds(on) | 0.06Ω | Lavt ✓ |
| Vgs(th) | -2V til -4V | Kompatibel med 9V gate drive ✓ |

**Fordele:**
- Meget lav Rds(on) → minimal effekttab
- Høj strømkapacitet → god margin
- Standard TO-220 pakke

### 7.2 IRL530 (N-kanal Logic Level)

| Parameter | Værdi | Krav |
|-----------|-------|------|
| Vds | 100V | > 9V ✓ |
| Id | 15A | > 0.1A ✓ |
| Rds(on) | 0.16Ω | Acceptabelt ✓ |
| Vgs(th) | 1V til 2V | **Logic Level** ✓ |

**Fordele:**
- Logic-level gate → kan drives direkte fra 5V MCU
- Standard TO-220 pakke
- Bruges både som switch og gate driver

### 7.3 Gate Pull-up Modstande (5 kΩ)

Funktioner:
1. Holder P-kanal MOSFETs slukket når driver N-kanal er slukket
2. Begrænser gate strøm under switching

**Gate charge tid:**
$$\tau = R \times C_{iss} = 5000 \times 1800pF = 9\ \text{µs}$$

Ved 2 kHz (500 µs periode) er dette acceptabelt.

---

## 8. Arduino Nano Integration

### 8.1 Pin Konfiguration

| Signal | Arduino Pin | MOSFET | Funktion |
|--------|-------------|--------|----------|
| V1 | D9 (PB1) | M5 → M2 | Venstre high-side |
| V2 | D10 (PB2) | M6 → M1 | Højre high-side |
| V3 | D9 (PB1) | M3 | Højre low-side |
| V4 | D10 (PB2) | M4 | Venstre low-side |

> [!note] Simplified Drive
> V1=V3 og V2=V4 kan forbindes sammen, så kun 2 PWM signaler er nødvendige.

### 8.2 Timer Konfiguration

For 2 kHz H-bro drive fra Timer1:

```c
void hbridge_init(void) {
    // Pin D9 og D10 som output
    DDRB |= (1 << PB1) | (1 << PB2);

    // Timer1: Phase-correct PWM, TOP = ICR1
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << CS11);  // Prescaler 8

    // 2 kHz: 16MHz / 8 / 1000 / 2 = 1000 (phase-correct halves freq)
    ICR1 = 1000;

    // 50% duty cycle, komplementære outputs
    OCR1A = 500;  // D9
    OCR1B = 500;  // D10 (inverteret via COM1B0)
}
```

### 8.3 Dead-Time Overvejelser

> [!warning] Shoot-Through Risiko
> Uden dead-time kan begge sider af H-broen være tændt samtidig → kortslutning!

**Anbefaling:** Tilføj 1-2 µs dead-time mellem skift:

```c
// Software dead-time (simpel metode)
#define DEAD_TIME_US 2

ISR(TIMER1_OVF_vect) {
    // Sluk alle MOSFETs først
    PORTB &= ~((1 << PB1) | (1 << PB2));
    _delay_us(DEAD_TIME_US);
    // Tænd nye side
    // ...
}
```

---

## 9. PCB Layout Anbefalinger

### 9.1 Kritiske Punkter

1. **Korte gate-forbindelser** - Minimer induktans i gate drive
2. **Bred power trace** - Høj strøm kræver bred kobber
3. **Dedikeret GND plane** - Reducer støj
4. **Bypass kondensatorer** - 100 nF tæt på hver MOSFET
5. **Termisk relief** - TO-220 pakker kan blive varme

### 9.2 Komponent Placering

```
┌─────────────────────────────────────────────┐
│                                             │
│    [M2]────────[L1]────────[M1]            │
│     │    IRF5305   6.33mH   IRF5305   │            │
│     │                           │            │
│    [M5]    [C1]    [R1]    [M6]            │
│   IRL530   10µF    35Ω   IRL530           │
│     │                           │            │
│    [M4]─────────────────────[M3]            │
│   IRL530                    IRL530           │
│     │           │           │            │
│    GND─────────GND─────────GND            │
│                                             │
│          [Arduino Nano]                     │
│            D9    D10                        │
└─────────────────────────────────────────────┘
```

---

## 10. Stykliste (BOM)

| Ref | Komponent | Værdi/Type | Pakke | Antal | Noter |
|-----|-----------|------------|-------|-------|-------|
| M1, M2 | MOSFET P-kanal | IRF5305PbF | TO-220 | 2 | High-side |
| M3, M4, M5, M6 | MOSFET N-kanal | IRL530 | TO-220 | 4 | Logic-level |
| L1 | Induktor | 6.33 mH | - | 1 | TX spole |
| R1 | Modstand | 35 Ω, 1W | Axial | 1 | Serie modstand |
| R2, R3 | Modstand | 5 kΩ | 0805 | 2 | Gate pull-up |
| C1 | Kondensator | 10 µF, 25V | Elektrolyt | 1 | DC blokering |
| C2, C3 | Kondensator | 100 nF | 0805 | 2 | Bypass |

---

## 11. Relaterede Dokumenter

- [[Power Budget Analysis|Strømbudget Analyse]]
- [[DFT Algorithm|DFT Algoritme]]
- [[../Guides/Code_Review|Firmware Kodegennemgang]]
- [[Coil Design|Spole Design]]

---

## 12. Teori Referencer (DTU Vault)

| Emne | Link | Relevans |
|------|------|----------|
| MOSFET Grundlag | [MOS transistor basics](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLecture%20Notes%2FMOS%20transistor%20basics) | Ids ligninger, Rds(on), switching |
| Analog Formler | [FORMULAS](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FExercises%2FHome%20Assignments%2F2%2FFORMULAS) | Transferfunktioner, frekvensrespons |
| Løsningseksempler | [Solution_Sheet](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FExercises%2FHome%20Assignments%2F2%2FSolution_Sheet) | W/L forhold, transkonduktans, impedans |

---

*TX Driver design til DTU 34621 Metaldetektor Projekt*
