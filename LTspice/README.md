# ⚡ LTspice Simulations

Circuit simulations for the VLF Metal Detector.

> 📚 **Theory:** [Op-Amp Design](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLTspice%20%26%20Kicad%2F02%20-%20Two-Stage%20CMOS%20Op-Amp) | [Filter Prototypes](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FFilter_Prototypes_Comparison)

## Structure

```
LTspice/
├── simulations/     # .asc simulation files
│   ├── tx_coil_driver.asc
│   ├── rx_amplifier.asc
│   ├── bandpass_filter.asc
│   └── full_analog_chain.asc
├── models/          # .lib and .mod component models
│   └── op_amp_models.lib
└── exports/         # Exported plots and data
    ├── bode_plots/
    └── transient/
```

## Circuits to Simulate

### 1. TX Coil Driver (Power Amplifier)
- [ ] PWM to sine conversion
- [ ] Current drive capability
- [ ] Power consumption from 9V

### 2. RX Coil Amplifier
- [ ] Gain calculation
- [ ] Noise analysis
- [ ] Bandwidth (centered on 2kHz)

### 3. Bandpass Filter
- [ ] Center frequency: 2 kHz
- [ ] Q factor selection
- [ ] Phase response

### 4. Full Analog Chain
- [ ] End-to-end simulation
- [ ] Phase shift verification
- [ ] Signal levels at each stage

## Simulation Checklist

| Circuit    | Schematic | Transient | AC Analysis | Notes |
| ---------- | --------- | --------- | ----------- | ----- |
| TX Driver  | ⬜         | ⬜         | ⬜           |       |
| RX Amp     | ⬜         | ⬜         | ⬜           |       |
| Filter     | ⬜         | ⬜         | ⬜           |       |
| Full Chain | ⬜         | ⬜         | ⬜           |       |

## LTspice Tips

### Useful Directives
```spice
.tran 0 10m 0 1u      ; Transient: 10ms, 1µs step
.ac dec 100 10 100k   ; AC: 10Hz to 100kHz, 100 pts/decade
.param Vcc=9          ; Parameter for supply voltage
```

### Measuring Phase
1. Run AC analysis
2. Add plot: `V(out)` and `V(in)`
3. Right-click → Add Trace → Phase

### Op-Amp Models
Use generic models or specific ones:
- `UniversalOpAmp2` - Generic
- `LM358` - Dual, rail-to-rail input
- `TL072` - Low noise JFET

## Related Documents
- [[Literature/op_amps_everyone.pdf|Op Amps for Everyone]]
- [[Literature/filters_v3_III_v1.pdf|Filter Design]]
- [[Literature/RLC_Circuits_and_Resonance.pdf|RLC Circuits]]

## Tags
#ltspice #simulation #analog #circuits
