# Report Export Assets

This folder contains assets prepared for export to the DTU 34621 VLF Metal Detector report repository (Overleaf).

## Purpose

This folder serves as a staging area for figures, schematics, code snippets, and data that will be included in the final project report. Assets placed here are ready for export to the report repository.

## Folder Structure

| Folder | Description | Export Destination |
|--------|-------------|-------------------|
| `figures/` | Plots, diagrams, photos, and other images | `report/figures/` |
| `schematics/` | Circuit schematics and PCB layouts | `report/figures/schematics/` |
| `code_snippets/` | Key code excerpts for the report | `report/code/` |
| `data/` | Measurement data, CSV files, tables | `report/data/` |

## Workflow for Adding Assets

1. **Create/Export the Asset**
   - Generate figures from simulation tools or measurement software
   - Export schematics as PDF or high-resolution PNG
   - Copy relevant code snippets to separate files
   - Save measurement data as CSV or formatted tables

2. **Place in Appropriate Folder**
   - Use the correct subfolder based on asset type
   - Follow the naming convention (see below)

3. **Run Export Script**
   - Execute `./export_to_report.sh <report_repo_path>` from the project root
   - Commit and push changes in the report repository

## Running the Export Script

From the project root directory:

```bash
# Export to sibling report repository
./export_to_report.sh ../34621-Report

# Export to custom path
./export_to_report.sh /path/to/report/repo
```

The script will:
- Copy all non-empty asset folders to the report repository
- Skip folders that only contain `.gitkeep` files
- Print instructions for committing changes

## Naming Convention

Use **lowercase with underscores** for all filenames. Names should be descriptive and include context.

### Examples

| Good | Bad |
|------|-----|
| `tx_driver_schematic.pdf` | `Schematic1.pdf` |
| `dft_phase_response.png` | `Figure 3.png` |
| `rx_amplifier_bode_plot.png` | `bode.PNG` |
| `power_consumption_data.csv` | `data.csv` |
| `dft_algorithm.c` | `code.c` |

### Recommended Prefixes

- `tx_` - Transmitter related
- `rx_` - Receiver related
- `dft_` - DFT/signal processing related
- `coil_` - Coil design related
- `power_` - Power supply/consumption related
- `test_` - Test results and measurements

## File Formats

| Asset Type | Preferred Format | Alternative |
|------------|-----------------|-------------|
| Schematics | PDF (vector) | PNG (300+ DPI) |
| Plots/Graphs | PDF (vector) | PNG (300+ DPI) |
| Photos | JPEG | PNG |
| Code | `.c`, `.h` | `.txt` |
| Data | CSV | TXT |

## Notes

- Vector formats (PDF) are preferred for schematics and plots as they scale without quality loss
- Ensure all figures have readable labels and appropriate font sizes
- Include units in axis labels for all plots
- Code snippets should be self-contained and well-commented
