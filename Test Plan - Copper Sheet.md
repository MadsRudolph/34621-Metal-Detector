# Test Plan: Copper Sheet (Non-Ferrous)

**Date:** 2026-01-19
**Tester:**
**Equipment:** Metal detector, 98K potentiometer, copper sheet
**Gain setting:** ~50-60% (optimal from previous test)

---

## Step 1: Calibrate
- [x] Set pot to **50-60%** position
- [x] No metal near coil
- [x] Press calibrate button

| Measurement | Value |
|-------------|-------|
| Amplitude   | 3     |
| Phase       | 90    |

---

## Step 2: Copper on Coil (0 cm)
- [x] Place copper sheet directly on coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 50    |
| Phase       | -40   |

---

## Step 3: Copper at 5 cm
- [x] Hold copper sheet ~5 cm above coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 36    |
| Phase       | -37   |

---

## Step 4: Copper at 10 cm
- [x] Hold copper sheet ~10 cm above coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 25    |
| Phase       | -25   |

---

## Comparison: Metal Cap vs Copper Sheet

| Test Object | Amplitude | Phase | Δ Phase from baseline |
|-------------|-----------|-------|----------------------|
| No metal (baseline) | 3 | 90° | 0° |
| Metal cap (ferro) | 11 | 120° | +30° |
| Copper sheet (non-ferro) | 50 | -40° | -130° |

---

## Expected Result

- **Ferrous metals** (steel, iron): Phase shift **positive** (towards +180°)
- **Non-ferrous metals** (copper, aluminum): Phase shift **negative** (towards -180°) or smaller positive shift

The copper sheet should show a **different phase response** than the metal cap if your ferro/non-ferro detection is working.

---

## Notes / Observations

### Konklusion

**Fase-detektion virker korrekt** - Testen bekræftede at fase-responsen er tydelig forskellig mellem ferro og non-ferro metaller:
- **Metal cap (ferro):** Fase går **positiv** (90° → 120° = +30°)
- **Kobber (non-ferro):** Fase går **negativ** (90° → -40° = -130°)

**Kobber giver stærkere signal** - Amplitude 50 vs 11 for metal cap. Dette skyldes kobbers høje ledningsevne som skaber stærke hvirvelstrømme.

**Afstandsdetektion virker** - Amplitude falder med afstand som forventet:
- 0 cm: Amplitude 50
- 5 cm: Amplitude 36
- 10 cm: Amplitude 25

### Software-fejl fundet og rettet

Under testen opdagede vi at klassificeringen var **inverteret** i koden:
- Koden sagde: negativt faseskift = ferro (FORKERT)
- Virkeligheden: negativt faseskift = non-ferro (KORREKT)

**Løsning:** Klassificeringslogikken blev rettet så:
- Positivt faseskift → FERRO (jern, stål)
- Negativt faseskift → NON-FERRO (kobber, aluminium)

### Tærskelværdier justeret

Den oprindelige magnitude-tærskel (+20) var for høj til at detektere svagere signaler. Sænket til 20% af baseline eller minimum +3 for bedre følsomhed.

