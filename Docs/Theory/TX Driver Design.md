# TX Driver Design

> [!abstract] Dokumentformål
> Dokumentation af H-bro TX driver kredsløb til metaldetektor sendespole.
> Design simuleret i Qspice.

---

## Indholdsfortegnelse

### H-Bro Design (Original)
1. [[#1. Kredsløbsoversigt|Kredsløbsoversigt]]
2. [[#2. Komponentliste|Komponentliste]]
3. [[#3. Driftsprincip|Driftsprincip]]
4. [[#4. Simuleringsparametre|Simuleringsparametre]]
5. [[#5. Elektriske Beregninger|Elektriske Beregninger]]
6. [[#6. Resonansovervejelser|Resonansovervejelser]]
7. [[#7. Komponent Valg Begrundelse|Komponent Valg Begrundelse]]
8. [[#8. Arduino Nano Integration|Arduino Nano Integration]]
9. [[#9. PCB Layout Anbefalinger|PCB Layout Anbefalinger]]
10. [[#10. Stykliste (BOM)|Stykliste (BOM)]]

### Push-Pull Design (Anbefalet) ⭐
11. [[#11. Push-Pull Driver Design (Anbefalet)|Push-Pull Driver Design]]
12. [[#12. Design Sammenligning: H-bro vs Push-Pull|Design Sammenligning]]
13. [[#13. Effektivitet og Batteriforbrug|Effektivitet og Batteriforbrug]]
14. [[#14. Fordele og Ulemper|Fordele og Ulemper]]
15. [[#15. Resonansberegninger for Push-Pull|Resonansberegninger]]
16. [[#16. Anbefaling|Anbefaling]]
17. [[#17. Push-Pull Tilslutninger og Byggevejledning|Tilslutninger og Byggevejledning]]
18. [[#18. Test Procedure for Push-Pull Driver|Test Procedure]]

### Referencer
19. [[#19. Relaterede Dokumenter|Relaterede Dokumenter]]
20. [[#20. Teori Referencer (DTU Vault)|Teori Referencer]]

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

## 11. Push-Pull Driver Design (Anbefalet)

> [!success] Læreranbefalet Design
> Dette push-pull kredsløb blev anbefalet af underviseren som et fungerende alternativ til H-broen. Det er simplere og kræver kun én kontrolsignal fra Arduino.

### 11.1 Kredsløbsoversigt

**Push-Pull (Totem-Pole) Driver** med 5 transistorer:
- 1× NPN power switch (BC337-16) - low-side
- 1× PNP power switch (BC327-16) - high-side
- 3× småsignal transistorer til gate/base driving

### 11.2 Komponentliste

| Ref | Komponent | Type | Funktion |
|-----|-----------|------|----------|
| Q1 | BC337-16 | NPN | Low-side power switch |
| Q2 | BC327-16 | PNP | High-side power switch |
| Q3 | BC847A | NPN | Input stage fra Arduino |
| Q4 | BC847A | NPN | Driver til Q2 |
| Q5 | BC857A | PNP | Driver til Q1 |
| D1, D2 | 1N4148 | Signal diode | Flyback beskyttelse |
| R1, R2, R4, R5, R6 | 10 kΩ | Modstand | Biasing |
| R3 | ~35 Ω | Modstand | Spole DC modstand |
| C1 | 1.0 µF | Kondensator | Resonans (beregnes ud fra L1) |
| L1 | 6.33-15 mH | Induktor | TX sendespole |
| V1 | 9V | Batteri | Forsyning (Rser ≈ 1Ω) |
| V2 | 0-5V | Arduino D9 | Input signal (2 kHz) |

### 11.3 Skematisk Diagram

```
                                    V1 (+9V)
                                       │
                                       │ Rser=1Ω
                                       │
              ┌────────────────────────┴────────────────────────┐
              │                                                  │
              │                                                  │
         ┌────┴────┐                                        ┌────┴────┐
         │   R1    │                                        │   R2    │
         │  10kΩ   │                                        │  10kΩ   │
         └────┬────┘                                        └────┬────┘
              │                                                  │
              │   ┌─────────────────────────────────────────┐    │
              │   │                                         │    │
              │   │    Q2 (BC327-16 PNP)                   │    │
              │   │         E                               │    │
              └───┼─────────●                               │    │
                  │         │                               │    │
                  │      ───┼───                            │    │
                  │     /   │   \                           │    │
                  └────●    │    ●──────────────────────────┼────┘
                       B    │    C                          │
                            │                               │
              ┌─────────────┴───────────────────────────────┤
              │                   OUTPUT                    │
              │                     │                       │
              │    ┌────────────────┼────────────────┐      │
              │    │                │                │      │
              │  ──┴──           ┌──┴──┐          ┌──┴──┐   │
              │  ──┬── C1        │     │          │     │   │
              │    │  (1µF)      │ L1  │          │ R3  │   │
              │    │             │6.33mH          │ 35Ω │   │
              │    │             │     │          │     │   │
              │    │             └──┬──┘          └──┬──┘   │
              │    │                │                │      │
              │    └────────────────┴────────────────┘      │
              │                     │                       │
              │            ▲ D1 (1N4148) ▲ D2               │
              │            │──────────────│                 │
              │            │              │                 │
              │            │              │                 │
              │    Q1 (BC337-16 NPN)      │                 │
              │         C                  │                 │
              │         ●──────────────────┘                 │
              │         │                                    │
              │      ───┼───                                 │
              │     /   │   \                                │
              └────●    │    ●───────────────────────────────┘
                   B    │    E
                        │
                        ▼ GND


  DRIVER STAGE:
  ═════════════

              V1 (+9V)
                 │
            ┌────┴────┐
            │   R4    │
            │  10kΩ   │
            └────┬────┘
                 │
                 ├───────────────────────────────► til Q2 base
                 │
       Q4 (BC847A NPN)
            C    │
            ●────┘
            │
         ───┼───
        /   │   \
  ┌────●    │    ●────────────────────┐
  │    B    │    E                    │
  │         │                         │
  │         ▼ GND                     │
  │                                   │
  │    Q3 (BC847A NPN)                │
  │         C                         │
  │         ●─────────────────────────┘
  │         │
  │      ───┼───
  │     /   │   \
  │  ──●    │    ●──────────────────────────► til Q5 base
  │    B    │    E                              via R6
  │    │    │
  │    │    ▼ GND
  │    │
  │ ┌──┴──┐
  │ │ R5  │
  │ │10kΩ │
  │ └──┬──┘
  │    │
  │    ▼
  │   V2 (Arduino D9: 2kHz, 0-5V)
  │
  │
  │    Q5 (BC857A PNP)
  │         E
  │         ●───────────────────────────┐
  │         │                           │
  │      ───┼───                        │
  │     /   │   \                       │
  └────●    │    ●                      │
       B    │    C                      │
            │    │                      │
            │    └──────────────────────┼──► til Q1 base
            │                           │
       ┌────┴────┐                 ┌────┴────┐
       │   R6    │                 │   R2    │
       │  10kΩ   │                 │  10kΩ   │
       └────┬────┘                 └────┬────┘
            │                           │
            ▼ GND                       │
                                        V1 (+9V)
```

### 11.4 Driftsprincip

#### V2 HIGH (5V) - Strøm ind i spole:

```
1. Q3 tænder (NPN får base strøm via R5)
2. Q4 slukker (base trækkes til GND via Q3)
3. Q2 tænder (PNP base trækkes lav via R4 til slukket Q4)
4. Q5 slukker (base forbundet til Q3 collector → høj)
5. Q1 slukker (base er høj via R2)
6. Strøm flyder: V1 → Q2 → LC tank → GND
```

#### V2 LOW (0V) - Strøm ud af spole:

```
1. Q3 slukker (ingen base strøm)
2. Q4 tænder (base får strøm via R4)
3. Q2 slukker (PNP base = VCC via Q4)
4. Q5 tænder (PNP base = lav via R6)
5. Q1 tænder (NPN base strøm via Q5 og R2)
6. Strøm flyder: LC tank → Q1 → GND
```

#### Resonans:

```
C1 + L1 danner resonant tank kredsløb
- Firkantbølge input → sinusbølge output
- Resonans ved f = 1/(2π√(LC))
- D1, D2 beskytter mod induktive spikes
```

### 11.5 Timing Diagram

```
         ◄──────── 500 µs (2 kHz) ────────►

V2 (D9): ┌────────────────┐              ┌────
    5V ──┤                │              │
    0V   └────────────────┴──────────────┘
         │◄─── 250 µs ───►│◄─ 250 µs ───►│

Q2 (PNP):┌────────────────┐              ┌────
    ON ──┤                │              │
   OFF   └────────────────┴──────────────┘

Q1 (NPN):                 ┌──────────────┐
    ON ───────────────────┤              │────
   OFF                    └──────────────┘

Output:  ┌────────────────┐              ┌────
   +9V ──┤   Q2 leder     │              │
    0V   │                │              │
   GND   └────────────────┴── Q1 leder ──┘

LC Tank: /\    /\    /\    /\    /\    /\
(sinus) /  \  /  \  /  \  /  \  /  \  /  \
       /    \/    \/    \/    \/    \/    \
```

---

## 12. Design Sammenligning: H-bro vs Push-Pull

### 12.1 Sammenligningstabel

| Aspekt | H-bro Design | Push-Pull Design |
|--------|--------------|------------------|
| **Antal transistorer** | 6 (2× P-MOSFET + 4× N-MOSFET) | 5 (2× BJT power + 3× BJT driver) |
| **Kontrolsignaler** | 2-4 (med dead-time) | 1 (direkte fra D9) |
| **Total komponentantal** | ~12 | ~14 |
| **Kompleksitet** | Høj | Lav |
| **Dead-time krav** | Kritisk nødvendig | Ikke nødvendig |
| **Shoot-through risiko** | Høj | Meget lav |
| **Output spændingsswing** | ±9V (18 Vpp) | 0 til +9V (9 Vpp) |
| **Maks effekt til spole** | Højere | Lavere (ca. halvdelen) |
| **Arduino kode ændringer** | Ja (kompleks PWM) | Nej (eksisterende D9 output) |
| **Debugbarhed** | Svær | Nem |
| **Konstruktionstid** | Lang | Kort |
| **Komponentpris** | Højere (power MOSFETs) | Lavere (standard BJTs) |
| **PCB plads** | Større (TO-220 MOSFETs) | Mindre (SMD BJTs muligt) |
| **Lærer godkendt** | Nej (ikke testet) | Ja ✓ |

### 12.2 Elektrisk Sammenligning

| Parameter | H-bro | Push-Pull |
|-----------|-------|-----------|
| Vout,pp | 18 V | 9 V |
| Icoil,peak (Z=87Ω) | 103 mA | 52 mA |
| Icoil,rms | 73 mA | 37 mA |
| Pcoil | 186 mW | 47 mW |
| Magnetfelt styrke | Højere | Lavere |

---

## 13. Effektivitet og Batteriforbrug

> [!info] Batteri Reference
> Duracell MN1604 (9V alkalisk): 580 mAh nominel, ~350 mAh effektiv ved 120 mA afladning til 6V cutoff.
> Se [[Power Budget Analysis|Strømbudget Analyse]] for detaljer.

### 13.1 Driver Effektivitet Sammenligning

| Parameter | H-bro (MOSFET) | Push-Pull (BJT) |
|-----------|----------------|-----------------|
| **Transistor type** | MOSFET | BJT |
| **Spændingsfald (leder)** | Rds(on) × I | Vce(sat) ≈ 0.3-0.5V |
| **Konduktions tab** | < 2 mW | ~30 mW |
| **Switching tab @ 2kHz** | < 0.5 mW | ~5 mW |
| **Gate/Base drive tab** | < 1 mW | ~15 mW |
| **Biasing tab** | ~5 mW | ~20 mW |
| **Total driver tab** | **< 10 mW** | **~70 mW** |
| **Driver effektivitet** | **>98%** | **~85%** |

#### 13.1.1 H-bro Tab Beregning

```
MOSFET Konduktions tab (IRF5305 + IRL530):
- High-side: P = I²rms × Rds(on) = 0.073² × 0.06Ω = 0.32 mW
- Low-side:  P = I²rms × Rds(on) = 0.073² × 0.16Ω = 0.85 mW
- Total konduktion: ~1.2 mW

Gate drive tab:
- Gate charge × Vgs × f = 50nC × 9V × 2kHz × 4 = 3.6 mW

Pull-up modstande (5kΩ til 9V):
- P = V²/R × duty = 81/5000 × 0.5 = 8 mW

Total H-bro driver tab: ~13 mW
```

#### 13.1.2 Push-Pull Tab Beregning

```
BJT Konduktions tab (BC337 + BC327):
- Q1 (NPN): P = Vce(sat) × Ic × duty = 0.3V × 50mA × 0.5 = 7.5 mW
- Q2 (PNP): P = Vce(sat) × Ic × duty = 0.4V × 50mA × 0.5 = 10 mW
- Total power stage: ~18 mW

Driver transistorer (Q3, Q4, Q5):
- Estimated: ~10 mW

Biasing modstande (5× 10kΩ):
- R4 (når Q4 slukket): ~4 mW
- R1 (til Q2 base): ~4 mW
- Andre: ~8 mW
- Total biasing: ~16 mW

Total push-pull driver tab: ~44 mW
```

### 13.2 Effekt til Spole Sammenligning

| Parameter | H-bro | Push-Pull | Forskel |
|-----------|-------|-----------|---------|
| Forsyningsspænding | 9V | 9V | - |
| Output spændingsswing | ±9V (18 Vpp) | 0-9V (9 Vpp) | -50% |
| Spoleimpedans @ 2kHz | 87 Ω | 87 Ω | - |
| Spolestrøm (peak) | 103 mA | 52 mA | -50% |
| Spolestrøm (rms) | 73 mA | 37 mA | -50% |
| Effekt i spolemodstand | 186 mW | 47 mW | -75% |
| **Magnetfelt styrke** | **100%** | **50%** | **-50%** |

### 13.3 Total Strømforbrug fra Batteri

| Komponent | H-bro (mA) | Push-Pull (mA) |
|-----------|------------|----------------|
| **Elektronik baseline** | | |
| ATmega328P + periferier | 15 | 15 |
| CH340G USB chip | 3 | 3 |
| Power LED | 3 | 3 |
| AMS1117 quiescent | 5 | 5 |
| SSD1306 OLED | 12 | 12 |
| PCB lækage | 2 | 2 |
| **Subtotal elektronik** | **40** | **40** |
| | | |
| **TX Driver** | | |
| Spolestrøm (avg) | 80 | 45 |
| Driver tab | 1 | 8 |
| **Subtotal TX** | **81** | **53** |
| | | |
| **TOTAL SYSTEM** | **121 mA** | **93 mA** |

### 13.4 Batterilevetid Beregning

#### Duracell MN1604 Kapacitet vs. Afladningsrate

| Strøm (mA) | Køretid til 6V | Effektiv Kapacitet |
|------------|----------------|-------------------|
| 50 | 600 min | 500 mAh |
| 100 | 210 min | 350 mAh |
| 120 | ~175 min | 350 mAh |
| 150 | 120 min | 300 mAh |

#### Estimeret Køretid

**H-bro design (121 mA):**
$$t_{H-bro} = \frac{350\ \text{mAh}}{121\ \text{mA}} \times 60 = 174\ \text{min}$$

**Push-pull design (93 mA):**

Interpolering fra datablad (mellem 50 mA og 100 mA):
$$C_{eff} \approx 420\ \text{mAh ved 93 mA}$$

$$t_{push-pull} = \frac{420\ \text{mAh}}{93\ \text{mA}} \times 60 = 271\ \text{min}$$

### 13.5 Batterilevetid Oversigt

```
┌────────────────────────────────────────────────────────────────────┐
│              BATTERILEVETID SAMMENLIGNING                          │
│                  (Duracell MN1604, til 6V cutoff)                  │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  H-BRO DESIGN:                                                     │
│  ████████████████████████████████████████░░░░░░░░░░░░░░░░░░░░░░░░ │
│  │◄────────── 174 min (~2.9 timer) ──────────►│                   │
│  Strøm: 121 mA | Magnetfelt: 100% | Dybde: 100%                   │
│                                                                    │
│  PUSH-PULL DESIGN:                                                 │
│  ████████████████████████████████████████████████████████████░░░░ │
│  │◄──────────────── 271 min (~4.5 timer) ────────────────►│       │
│  Strøm: 93 mA | Magnetfelt: 50% | Dybde: 79%                      │
│                                                                    │
│  KRAV: Minimum 100 min ────────────────────────│                   │
│                         ✅ Begge opfylder krav                     │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

### 13.6 Effektivitets-Ydeevne Afvejning

| Aspekt | H-bro | Push-Pull | Vinder |
|--------|-------|-----------|--------|
| Driver effektivitet | >98% | ~85% | H-bro |
| Batterilevetid | 174 min | 271 min | **Push-Pull** |
| Magnetfelt styrke | 100% | 50% | H-bro |
| Detektionsdybde* | 100% | 79% | H-bro |
| Margin over 100 min krav | +74 min | +171 min | **Push-Pull** |
| Strøm tilgængelig for andre | 0 mA | 28 mA | **Push-Pull** |

*Detektionsdybde skalerer med $B^{1/3}$, så 50% felt → 79% dybde

### 13.7 Optimeringsmuligheder

#### H-bro Strømbesparelse
| Ændring | Besparelse | Ny Køretid |
|---------|------------|------------|
| Fjern power LED | 3 mA → 118 mA | ~185 min |
| Sluk OLED under søgning | 12 mA → 106 mA | ~210 min |
| Begge | 15 mA → 103 mA | ~220 min |

#### Push-Pull Strømbesparelse
| Ændring | Besparelse | Ny Køretid |
|---------|------------|------------|
| Fjern power LED | 3 mA → 90 mA | ~290 min |
| Sluk OLED under søgning | 12 mA → 78 mA | ~350 min |
| Begge | 15 mA → 75 mA | ~380 min |

### 13.8 Konklusion: Hvad skal vælges?

> [!tip] Beslutningsguide
>
> **Vælg H-bro hvis:**
> - Maksimal detektionsdybde er kritisk
> - 174 min køretid er tilstrækkelig
> - Kompleksitet ikke er en bekymring
>
> **Vælg Push-Pull hvis:**
> - 79% detektionsdybde er acceptabel
> - Længere batterilevetid ønskes
> - Simplere konstruktion foretrækkes
> - Projekttid er begrænset (anbefalet for DTU 34621)

### 13.9 Energieffektivitet Sammenfatning

| Metric | H-bro | Push-Pull |
|--------|-------|-----------|
| Energi fra batteri (100 min) | 182 Wh | 140 Wh |
| Energi til magnetfelt (100 min) | 11.2 Wh | 2.8 Wh |
| System effektivitet | 6.2% | 2.0% |
| Driver effektivitet | >98% | ~85% |
| **Energi per detektion*** | Bedre | - |

*Antager samme antal detektioner, men H-bro har 26% større rækkevidde

---

## 14. Fordele og Ulemper

### 14.1 H-bro Fordele
- ✅ Fuld bro spændingsswing (±9V over spolen)
- ✅ Højere effektlevering mulig
- ✅ Veldokumenteret topologi
- ✅ MOSFET = lavere konduktions tab

### 14.2 H-bro Ulemper
- ❌ Kræver 4 præcist timede kontrolsignaler
- ❌ Dead-time nødvendig for at undgå shoot-through
- ❌ Flere transistorer = flere fejlpunkter
- ❌ Kompleks gate drive til high-side P-MOSFETs
- ❌ Svær at debugge
- ❌ Arduino kode skal modificeres

### 14.3 Push-Pull Fordele
- ✅ Kun 1 kontrolsignal (direkte fra Arduino D9)
- ✅ Ingen dead-time management nødvendig
- ✅ Simplere kredsløb, færre komponenter
- ✅ Meget lav shoot-through risiko
- ✅ Nemmere at debugge og bygge
- ✅ **Lærer-godkendt design**
- ✅ Fungerer med eksisterende Arduino kode (ingen ændringer)
- ✅ Standard BJT komponenter (billige, nemme at skaffe)

### 14.4 Push-Pull Ulemper
- ❌ Halvdelen af spændingsswing (0-9V vs ±9V)
- ❌ Lavere maksimal effekt til spolen
- ❌ Asymmetrisk drive (kan kræve DC-blokering)
- ❌ BJT = højere konduktions tab end MOSFET

---

## 15. Resonansberegninger for Push-Pull

### 15.1 Resonansfrekvens Formel

For en LC tank ved resonans:

$$f_{res} = \frac{1}{2\pi\sqrt{LC}}$$

Omskrevet til at finde C:

$$C = \frac{1}{4\pi^2 f^2 L} = \frac{1}{(2\pi f)^2 L}$$

### 15.2 Beregningseksempler

**Eksempel 1: L1 = 6.33 mH (eksisterende spole)**

$$C = \frac{1}{(2\pi \times 2000)^2 \times 0.00633} = \frac{1}{158 \times 10^6 \times 0.00633}$$

$$C = \frac{1}{1.0 \times 10^6} = 1.0\ \mu\text{F}$$

**Eksempel 2: L1 = 15 mH (fra Coil Design dokument)**

$$C = \frac{1}{(2\pi \times 2000)^2 \times 0.015} = \frac{1}{158 \times 10^6 \times 0.015}$$

$$C = \frac{1}{2.37 \times 10^6} = 0.42\ \mu\text{F}$$

### 15.3 Anbefalet Kondensator

| Spole Induktans | Resonans C ved 2 kHz | Praktisk Værdi |
|-----------------|---------------------|----------------|
| 6.33 mH | 1.0 µF | 1.0 µF MKP/MKT |
| 10 mH | 0.63 µF | 0.68 µF MKP/MKT |
| 15 mH | 0.42 µF | 0.47 µF MKP/MKT |

> [!warning] Kondensator Krav
> - Film kondensator (MKP/MKT) anbefales - IKKE elektrolyt
> - Spændingsrating: minimum 50V (Q-faktor kan forstærke!)
> - Lav ESR for bedre Q-faktor

### 15.4 Q-faktor Beregning

$$Q = \frac{X_L}{R} = \frac{2\pi f L}{R_{coil}}$$

For L = 6.33 mH, R = 35 Ω:

$$Q = \frac{2\pi \times 2000 \times 0.00633}{35} = \frac{79.5}{35} = 2.3$$

**Spændingsforstærkning ved resonans:**

$$V_{peak} = Q \times V_{in} = 2.3 \times 9V = 20.7V$$

---

## 16. Anbefaling

> [!success] Anbefaling: Brug Push-Pull Design
>
> Baseret på sammenligning og projektets rammebetingelser anbefales **push-pull designet** af følgende grunde:
>
> 1. **Tidsbegrænsning**: Push-pull kan bygges og testes hurtigere
> 2. **Simplere debugging**: Én kontrolsignal gør fejlfinding nemmere
> 3. **Eksisterende kode**: Fungerer direkte med Arduino D9 output - ingen firmware ændringer
> 4. **Lærer-godkendt**: Designet er verificeret af underviseren
> 5. **Lavere risiko**: Ingen shoot-through bekymringer
> 6. **Tilstrækkelig effekt**: Selvom effekten er lavere, er det tilstrækkeligt til at generere detekterbart magnetfelt

### 16.1 Fremgangsmåde

1. Byg push-pull driver først på breadboard
2. Test med oscilloskop
3. Hvis mere effekt er nødvendig, kan H-bro implementeres senere

---

## 17. Push-Pull Tilslutninger og Byggevejledning

### 17.1 Arduino Tilslutninger

| Signal | Arduino Pin | Kredsløb Tilslutning | Noter |
|--------|-------------|---------------------|-------|
| TX input | D9 (PB1) | V2 via R5 (10kΩ) | 2 kHz firkantbølge |
| GND | GND | Kredsløb GND | Fælles reference |
| (Valgfri) | 5V | - | Kan forsyne driver stage |

### 17.2 Breadboard Opsætning

```
Breadboard Layout:
═══════════════════

    +9V rail ─────────────────────────────────────
         │
         ├── R1 (10k) ── Q2 (BC327 E)
         ├── R2 (10k) ── Q1 (BC337 base via Q5)
         └── R4 (10k) ── Q4 (BC847 C)

    Spole section:
         Q2 C ──┬── D1 cathode ── D2 anode ── Q1 C
                │
                ├── C1 ─┬─ L1 ─┬─ R3 (35Ω)
                │       │      │
                GND     └──────┘

    Driver section:
         Arduino D9 ── R5 (10k) ── Q3 base
         Q3 E ── GND
         Q3 C ── Q4 base, Q5 base via R6

    GND rail ─────────────────────────────────────
```

### 17.3 Bygge Noter

1. **Hold ledninger korte** - minimer parasitisk induktans
2. **Tilføj afkoblingskondensator** - 100 nF tæt på 9V forsyning
3. **Test uden spole først** - brug 100Ω modstand som load
4. **Kontroller transistor orientering** - E, B, C pinout varierer!
5. **Brug sokler** - gør det nemt at udskifte transistorer

### 17.4 Komponent Substitutioner

| Original | Alternativ | Noter |
|----------|------------|-------|
| BC337-16 | BC337-25, BC337-40 | Højere hFE OK |
| BC327-16 | BC327-25, BC327-40 | Højere hFE OK |
| BC847A | BC547, 2N3904 | Through-hole alternativ |
| BC857A | BC557, 2N3906 | Through-hole alternativ |
| 1N4148 | 1N4448, 1N914 | Hurtig signal diode |

---

## 18. Test Procedure for Push-Pull Driver

### 18.1 Trin-for-Trin Test

#### Trin 1: Visuel Inspektion
- [ ] Kontroller alle forbindelser
- [ ] Verificer transistor orientering
- [ ] Check for kortslutninger

#### Trin 2: Strømforsyning Test (uden Arduino)
- [ ] Tilslut 9V batteri
- [ ] Mål strømforbrug: Skal være < 5 mA (quiescent)
- [ ] Hvis strømmen er høj → fejl i kredsløb

#### Trin 3: Input Signal Test
- [ ] Tilslut Arduino D9
- [ ] Kør eksisterende TX kode (2 kHz output)
- [ ] Mål med oscilloskop på D9: Skal se 0-5V firkant

#### Trin 4: Output Test (uden LC tank)
- [ ] Tilslut 100Ω testmodstand i stedet for LC
- [ ] Mål over modstanden med oscilloskop
- [ ] Forventet: 0-9V firkantbølge ved 2 kHz

#### Trin 5: LC Tank Test
- [ ] Tilslut C1 og L1
- [ ] Mål over LC tank med oscilloskop
- [ ] Forventet: Sinusbølge ved 2 kHz
- [ ] Amplitude afhænger af Q-faktor

### 18.2 Forventede Bølgeformer

```
Test punkt: Q2/Q1 output (før C1)
──────────────────────────────────
      ┌────────┐        ┌────────┐
 +9V ─┤        │        │        │
      │        │        │        │
  0V ─┴────────┴────────┴────────┴─
      │◄──500µs──►│
      (2 kHz firkant)


Test punkt: Over LC tank (efter C1)
──────────────────────────────────
        ╱╲      ╱╲      ╱╲
  +V ──╱  ╲    ╱  ╲    ╱  ╲──
      ╱    ╲  ╱    ╲  ╱    ╲
  0V ─      ╲╱      ╲╱      ╲╱─
       (2 kHz sinus, amplitude = Q × Vin)
```

### 18.3 Fejlfinding

| Symptom | Mulig Årsag | Løsning |
|---------|-------------|---------|
| Ingen output | Q3 virker ikke | Check R5, D9 signal |
| Kun høj output | Q1 virker ikke | Check Q5, R2 forbindelser |
| Kun lav output | Q2 virker ikke | Check Q4, R4 forbindelser |
| Høj quiescent strøm | Shoot-through | Check transistor pinout |
| Forvrænget sinus | Forkert C1 værdi | Beregn C for resonans |
| Lav amplitude | Lav Q-faktor | Normal - afhænger af R_coil |

### 18.4 Målinger at Dokumentere

| Måling | Forventet | Målt | OK? |
|--------|-----------|------|-----|
| Quiescent strøm | < 5 mA | | |
| D9 signal (Vpp) | 5V | | |
| D9 frekvens | 2 kHz | | |
| Output uden LC (Vpp) | 9V | | |
| Output med LC (Vpp) | ~20V | | |
| LC resonansfrekvens | 2 kHz | | |

### 18.5 Stykliste (BOM) - Push-Pull Design

| Ref | Komponent | Værdi/Type | Pakke | Antal | Noter |
|-----|-----------|------------|-------|-------|-------|
| Q1 | BJT NPN | BC337-16 | TO-92 | 1 | Low-side power |
| Q2 | BJT PNP | BC327-16 | TO-92 | 1 | High-side power |
| Q3 | BJT NPN | BC847A | SOT-23 | 1 | Input stage |
| Q4 | BJT NPN | BC847A | SOT-23 | 1 | Q2 driver |
| Q5 | BJT PNP | BC857A | SOT-23 | 1 | Q1 driver |
| D1, D2 | Diode | 1N4148 | DO-35 | 2 | Flyback beskyttelse |
| R1, R2, R4, R5, R6 | Modstand | 10 kΩ | 0805 | 5 | Biasing |
| R3 | Modstand | 35 Ω, 0.5W | Axial | 1 | Spole modstand |
| C1 | Kondensator | 1.0 µF, 50V | MKP/MKT | 1 | Resonans |
| C2 | Kondensator | 100 nF | 0805 | 1 | Bypass |
| L1 | Induktor | 6.33 mH | - | 1 | TX spole |

**Estimeret pris:** < 50 DKK (ekskl. spole)

---

## 19. Relaterede Dokumenter

- [[Power Budget Analysis|Strømbudget Analyse]]
- [[DFT Algorithm|DFT Algoritme]]
- [[../Guides/Code_Review|Firmware Kodegennemgang]]
- [[Coil Design|Spole Design]]

---

## 20. Teori Referencer (DTU Vault)

| Emne | Link | Relevans |
|------|------|----------|
| MOSFET Grundlag | [MOS transistor basics](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLecture%20Notes%2FMOS%20transistor%20basics) | Ids ligninger, Rds(on), switching |
| Analog Formler | [FORMULAS](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FExercises%2FHome%20Assignments%2F2%2FFORMULAS) | Transferfunktioner, frekvensrespons |
| Løsningseksempler | [Solution_Sheet](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FExercises%2FHome%20Assignments%2F2%2FSolution_Sheet) | W/L forhold, transkonduktans, impedans |

---

*TX Driver design til DTU 34621 Metaldetektor Projekt*
