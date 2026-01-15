# LTspice & QSPICE Simuleringer

Kredsløbssimuleringer til VLF Metaldetektoren.

> **Teori:** [Op-Amp Design](obsidian://open?vault=Obsidian&file=Courses%2FIntegrated%20Analog%20Electronics%2FLTspice%20%26%20Kicad%2F02%20-%20Two-Stage%20CMOS%20Op-Amp) | [Filter Prototyper](obsidian://open?vault=Obsidian&file=Courses%2FDSP%2FFormulas%2FFilter_Prototypes_Comparison)

## Simuleringsværktøjer

| Værktøj | Anvendelse | Mappe |
|---------|------------|-------|
| LTspice | Generelle simuleringer | `LTspice/` |
| QSPICE | H-bro driver (bedre MOSFET modeller) | `QSPICE/` |

## Status

| Kredsløb | Skema | Transient | AC Analyse | Status |
|----------|-------|-----------|------------|--------|
| TX H-bro Driver | ✅ | ✅ | ✅ | **Færdig** (QSPICE) |
| RX Forstærker | ⬜ | ⬜ | ⬜ | Afventer |
| Båndpasfilter | ⬜ | ⬜ | ⬜ | Afventer |
| Fuld Analog Kæde | ⬜ | ⬜ | ⬜ | Afventer |

## TX H-bro Driver (QSPICE) ✅

H-bro MOSFET driver til TX spole er fuldt simuleret og verificeret.

### Nøgleresultater
- **Effektivitet:** ~90%
- **Output:** 2 kHz firkantbølge
- **P-MOSFET:** IRF5305PbF (2 stk, højside)
- **N-MOSFET:** IRL530 (2 stk, lavside)
- **Strømforbrug:** Inden for power budget

Se: [TX Driver Design.md](../Docs/Theory/TX%20Driver%20Design.md) for fuld analyse.

## Planlagte Simuleringer

### 1. RX Spole Forstærker
- [ ] Forstærkningsberegning
- [ ] Støjanalyse
- [ ] Båndbredde (centreret på 2kHz)

### 2. Båndpasfilter
- [ ] Centerfrekvens: 2 kHz
- [ ] Q-faktor valg
- [ ] Faserespons

### 3. Fuld Analog Kæde
- [ ] End-to-end simulering
- [ ] Faseforskel verifikation
- [ ] Signalniveauer ved hvert trin

## LTspice Tips

### Nyttige Direktiver
```spice
.tran 0 10m 0 1u      ; Transient: 10ms, 1µs step
.ac dec 100 10 100k   ; AC: 10Hz til 100kHz, 100 pkt/dekade
.param Vcc=9          ; Parameter for forsyningsspænding
```

### Måling af Fase
1. Kør AC analyse
2. Tilføj plot: `V(out)` og `V(in)`
3. Højreklik → Add Trace → Phase

### Op-Amp Modeller
Brug generiske modeller eller specifikke:
- `UniversalOpAmp2` - Generisk
- `LM358` - Dual, rail-to-rail input
- `TL072` - Lav støj JFET

## Mappestruktur

```
LTspice/
├── simulations/      # .asc simuleringsfiler
├── models/           # .lib og .mod komponentmodeller
└── exports/          # Eksporterede plots og data
    ├── bode_plots/
    └── transient/

QSPICE/
└── [H-bro simuleringer]
```

## Relaterede Dokumenter

- [TX Driver Design.md](../Docs/Theory/TX%20Driver%20Design.md) - H-bro analyse
- [Power Budget Analysis.md](../Docs/Theory/Power%20Budget%20Analysis.md) - Strømforbrug
- [[../Literature/op_amps_everyone.pdf|Op Amps for Everyone]]
- [[../Literature/RLC_Circuits_and_Resonance.pdf|RLC Kredsløb]]

#ltspice #qspice #simulering #analog #kredsløb
