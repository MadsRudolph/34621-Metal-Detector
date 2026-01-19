# Kalibrering af Metal Klassificerings Tærskel

**Dato:** 2026-01-19
**Endelig tærskel:** -70

---

## Baggrund

Metaldetektoren klassificerer metaller som ferro (jern, stål) eller non-ferro (aluminium, kobber) baseret på faseskift fra den kalibrerede baseline.

**Oprindelig antagelse:** Ferromagnetiske metaller giver positivt faseskift, non-ferro giver negativt faseskift.

**Problem:** I praksis gav begge metaltyper negative faseskift. Dog gav ferromagnetiske metaller *mindre* negative skift (tættere på baseline) end non-ferro metaller.

---

## Empiriske Testresultater

| Metal | Fase | Magnitude | Klassificering |
|-------|------|-----------|----------------|
| Jern | -55 | 132 | Ferro |
| Lille metalkapsel | +23 | 46 | Ferro |
| Aluminiumsplade | -142 | 150 | Non-Ferro |
| Kobberplade | -142 | 154 | Non-Ferro |
| 10 krone mønt | -65 | 3 | Ikke klassificeret (svagt signal) |

---

## Analyse

**Ferro område:** +23 til -55
**Non-ferro område:** -142

**Afstand:** -55 til -142 (87 graders separation)

En tærskel på **-70** ligger midt i dette interval:
- Fase > -70 → Ferro
- Fase <= -70 → Non-Ferro

---

## Implementation

Fil: `Code/src/detection.c`

```c
// Klassificer baseret på faseskift
// Ferro (jern): fase tættere på baseline (lille negativt skift)
// Non-ferro (aluminium, kobber): fase langt fra baseline (stort negativt skift)
// Tærskel baseret på empiriske målinger: Fe=-55, Al=-142
if (phase_diff > -70) {
    return METAL_FERRO;
}
return METAL_NONFERRO;
```

---

## Noter

- 10 krone mønten (cupronikkel legering) blev ikke klassificeret pga. lav magnitude (3), ikke pga. forkert tærskel
- Tærsklen kan kræve justering ved andre spolekonfigurationer eller miljøer
- Kalibrering i luft (D3 knap) er påkrævet før brug
