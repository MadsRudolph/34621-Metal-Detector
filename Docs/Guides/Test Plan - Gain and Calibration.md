# Test Plan: Gain & Calibration

**Date:** 2026-01-19
**Tester:**
**Equipment:** Metal detector, 98K potentiometer, metal cap test object

---

## Step 1: Minimum Gain (Baseline)
- [x] Turn pot fully **counter-clockwise** (minimum gain)
- [x] No metal near coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 1     |
| Phase       | 60    |

---

## Step 2: Calibrate at Minimum Gain
- [x] Press calibrate button (no metal near coil)
- [x] Wait for calibration to complete

| Measurement | Value |
|-------------|-------|
| Amplitude   | 1     |
| Phase       | 60    |

---

## Step 3: Metal Test at Minimum Gain
- [x] Place metal cap directly on coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 1     |
| Phase       | 60    |

---

## Step 4: Medium Gain (~50%)
- [x] Turn pot to **middle position**
- [x] Remove metal, no metal near coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 3     |
| Phase       | 90    |

---

## Step 5: Calibrate at Medium Gain
- [x] Press calibrate button (no metal near coil)

| Measurement | Value |
|-------------|-------|
| Amplitude   | 3     |
| Phase       | 90    |

---

## Step 6: Metal Test at Medium Gain
- [x] Place metal cap directly on coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 11    |
| Phase       | 120   |

---

## Step 7: High Gain (~80%)
- [x] Turn pot to **80% position**
- [x] Remove metal, no metal near coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 4     |
| Phase       | 100   |

---

## Step 8: Calibrate at High Gain
- [x] Press calibrate button (no metal near coil)

| Measurement | Value |
|-------------|-------|
| Amplitude   | 4     |
| Phase       | 100   |

---

## Step 9: Metal Test at High Gain
- [x] Place metal cap directly on coil

| Measurement | Value |
|-------------|-------|
| Amplitude   | 14    |
| Phase       | 123   |

---

## Step 10: Optimal Gain - Distance Test
- [ ] Find gain where no-metal reading is stable (not noisy)
- [ ] Calibrate at this gain setting

**Optimal pot position:** ____%

| Distance | Amplitude | Phase | Detected? |
|----------|-----------|-------|-----------|
| 0 cm     |           |       |           |
| 5 cm     |           |       |           |
| 10 cm    |           |       |           |
| 15 cm    |           |       |           |

---

## Notes / Observations

### Konklusion

**Minimum gain er for lav** - Ved minimum gain (amplitude=1) kunne detektoren slet ikke registrere metal. Amplitude og fase forblev uændret med metal på spolen.

**50% og 80% gain virker begge** - Begge indstillinger viste tydelig amplitude stigning og faseskift når metal var til stede:
- 50% gain: Amplitude 3→11 (3.7x stigning), fase +30°
- 80% gain: Amplitude 4→14 (3.5x stigning), fase +23°

**Anbefalet driftsindstilling: 50-60% gain**
- God detekteringsfølsomhed
- Tilstrækkelig hovedrum til større metalobjekter
- Lavere støjniveau end ved højere gain

### Hardware ændringer under test

Oprindeligt blev en 470K potentiometer brugt, men dette gav alt for høj forstærkning (op til 4700x). Signalet klippede kraftigt ved selv lave pot-indstillinger.

**Løsning:** Skiftet til 98K potentiometer, som giver en mere brugbar forstærkningsområde (1x til ~981x) hvor hele potentiometerets rotation kan udnyttes.

