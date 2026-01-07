# DFT Algorithm

> 📚 **Theory:** [DSP-Bible](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FDSP-Bible) | [Windowing Guide](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FFIR_Windowing_Complete_Guide) | [Week 1-4](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FWeek%201-4)

The optimized single-bin Discrete Fourier Transform used in our metal detector.

---

## The Optimization Trick

> [!tip] 4× Oversampling Magic
> Since we sample at **exactly 4× the signal frequency** (8 kHz sampling, 2 kHz signal), the sine and cosine values become trivial - **no trigonometric calculations needed!**

| Sample n | cos(2πn/4) | sin(2πn/4) |
|----------|------------|------------|
| 0 | +1 | 0 |
| 1 | 0 | +1 |
| 2 | -1 | 0 |
| 3 | 0 | -1 |
| 4 | +1 | 0 |
| ... | ... | ... |

---

## Algorithm

```c
int32_t real_sum = 0;
int32_t imag_sum = 0;

for (int i = 0; i < 64; i += 4) {
    // Real (cosine): multiply by {+1, 0, -1, 0}
    real_sum += samples[i];      // × +1
    real_sum -= samples[i+2];    // × -1

    // Imaginary (sine): multiply by {0, +1, 0, -1}
    imag_sum += samples[i+1];    // × +1
    imag_sum -= samples[i+3];    // × -1
}

// Magnitude and phase
float magnitude = sqrt(real_sum² + imag_sum²);
float phase_rad = atan2(-imag_sum, real_sum);
float phase_deg = phase_rad * 180 / PI;
```

---

## Why 32-bit Integers?

> [!warning] Integer Overflow Prevention
> With 64 samples at 12-bit resolution (max 4095):
> - Worst case sum: 64 × 4095 = **262,080**
> - This exceeds 16-bit range (65,535)
> - But fits easily in 32-bit signed int

---

## Normalization

The raw DFT magnitude needs scaling to percentage:

```c
#define DFT_NORMALIZATION_FACTOR (100.0f / (CYCLES_PER_BUFFER * ADC_MAX_VALUE * 141.0f))
```

> [!info] Normalization Constants
> - `CYCLES_PER_BUFFER` = 16 (64 samples / 4 samples per cycle)
> - `ADC_MAX_VALUE` = 4095
> - 141 = empirical calibration constant

---

## Phase Interpretation

> [!summary] Metal Classification by Phase
> | Phase Range | Metal Type | Physical Reason |
> |-------------|------------|-----------------|
> | < 65° | **Ferrous** | High magnetic permeability (μr >> 1) |
> | > 65° | **Non-ferrous** | Eddy currents dominate |

---

## Related
- [[Literature/The_Discrete_Fourier_Transform.pdf|DFT Theory]]
- [[Code/src/signal/signal_analyzer.c|Implementation]]
- [[Phase Detection]]

---
#theory #dsp #dft
