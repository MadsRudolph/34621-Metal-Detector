#!/usr/bin/env python3
"""
Regenerate battery discharge graph from CSV with smoothed voltage curve.
"""

import csv
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from pathlib import Path

# Configuration (must match original test)
TEST_NAME = "Metal Detector Battery Discharge Test"
VOLTAGE_THRESHOLD_V = 6.0
PASS_RUNTIME_MIN = 100

# Smoothing window size (number of samples to average)
SMOOTHING_WINDOW = 30

def load_csv(csv_path):
    """Load data from CSV file, skipping comment lines."""
    data = []
    with open(csv_path, 'r') as f:
        for line in f:
            if line.startswith('#'):
                continue
            break
        reader = csv.DictReader(f, fieldnames=['elapsed_seconds', 'elapsed_minutes', 'voltage_V'])
        # Skip header if present
        first = next(reader, None)
        if first and first['elapsed_seconds'] != 'elapsed_seconds':
            data.append({
                'elapsed_minutes': float(first['elapsed_minutes']),
                'voltage_V': float(first['voltage_V'])
            })
        for row in reader:
            data.append({
                'elapsed_minutes': float(row['elapsed_minutes']),
                'voltage_V': float(row['voltage_V'])
            })
    return data

def smooth_data(values, window_size):
    """Apply moving average filter."""
    if len(values) < window_size:
        return values
    kernel = np.ones(window_size) / window_size
    smoothed = np.convolve(values, kernel, mode='valid')
    return smoothed

def generate_graph(csv_path, data):
    """Generate graph with smoothed voltage curve."""

    times_min = np.array([d['elapsed_minutes'] for d in data])
    voltages_raw = np.array([d['voltage_V'] for d in data])

    # Apply smoothing
    voltages = smooth_data(voltages_raw, SMOOTHING_WINDOW)
    # Trim time array to match smoothed data
    trim = (len(voltages_raw) - len(voltages)) // 2
    times_min_smooth = times_min[trim:trim + len(voltages)]

    # Statistics (use raw data for accuracy)
    start_v = voltages_raw[0]
    end_v = voltages_raw[-1]
    total_runtime = times_min[-1]
    avg_rate = ((start_v - end_v) * 1000) / total_runtime if total_runtime > 0 else 0
    passed = total_runtime >= PASS_RUNTIME_MIN
    result = "PASS" if passed else "FAIL"

    # Styling
    COLOR_PRIMARY = '#2563eb'
    COLOR_THRESHOLD = '#dc2626'
    COLOR_PASS = '#16a34a'
    COLOR_BG = '#f8fafc'
    COLOR_GRID = '#e2e8f0'
    COLOR_FAIL_ZONE = '#fee2e2'
    COLOR_PASS_ZONE = '#dcfce7'

    plt.style.use('default')
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=150)
    fig.patch.set_facecolor('white')
    ax.set_facecolor(COLOR_BG)

    # Background zones
    max_x = max(total_runtime * 1.1, PASS_RUNTIME_MIN * 1.2, 1)
    ax.axvspan(0, PASS_RUNTIME_MIN, color=COLOR_FAIL_ZONE, alpha=0.3, label='Fail Zone')
    ax.axvspan(PASS_RUNTIME_MIN, max_x, color=COLOR_PASS_ZONE, alpha=0.3, label='Pass Zone')

    # Smoothed discharge curve
    ax.fill_between(times_min_smooth, voltages, 0, color=COLOR_PRIMARY, alpha=0.1)
    ax.plot(times_min_smooth, voltages, color=COLOR_PRIMARY, linewidth=2.5,
            label=f'Battery Voltage (smoothed, n={SMOOTHING_WINDOW})', zorder=10)

    # Threshold and pass lines
    ax.axhline(y=VOLTAGE_THRESHOLD_V, color=COLOR_THRESHOLD, linestyle='--',
               linewidth=1.5, alpha=0.8, zorder=5, label=f'Threshold ({VOLTAGE_THRESHOLD_V}V)')
    ax.axvline(x=PASS_RUNTIME_MIN, color=COLOR_PASS, linestyle=(0, (5, 5)),
               linewidth=1.5, alpha=0.8, zorder=5)

    # Start/end markers (use smoothed values for visual consistency)
    ax.scatter(times_min_smooth[0], voltages[0], color=COLOR_PASS, edgecolors='white',
               s=120, zorder=15, marker='o', linewidths=2, label=f'Start: {start_v:.2f}V')
    ax.scatter(times_min_smooth[-1], voltages[-1], color=COLOR_THRESHOLD, edgecolors='white',
               s=120, zorder=15, marker='s', linewidths=2, label=f'End: {end_v:.2f}V')

    # Labels
    ax.set_xlabel('Time (minutes)', fontsize=11, fontweight='500', labelpad=10)
    ax.set_ylabel('Voltage (V)', fontsize=11, fontweight='500', labelpad=10)
    ax.set_title(f'{TEST_NAME}', fontsize=16, fontweight='bold', pad=25, loc='left')

    # Grid and spines
    ax.grid(True, axis='y', linestyle='-', color=COLOR_GRID, alpha=0.7, zorder=0)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_color('#cbd5e1')
    ax.spines['bottom'].set_color('#cbd5e1')

    # Axis limits
    ax.set_xlim(0, max_x)
    y_min = min(voltages) - 0.2
    y_max = max(voltages) + 0.2
    ax.set_ylim(max(0, y_min), y_max)
    ax.xaxis.set_major_locator(ticker.AutoLocator())
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.2f V'))

    # Summary box
    summary_text = (
        fr"$\mathbf{{SUMMARY}}$" "\n"
        f"Runtime: {total_runtime:.1f} min\n"
        f"Voltage Drop: {(start_v-end_v)*1000:.0f} mV\n"
        f"Avg Rate: {avg_rate:.1f} mV/min\n"
        fr"Status: $\mathbf{{{result}}}$"
    )
    props = dict(boxstyle='round,pad=0.8', facecolor='white', edgecolor='#e2e8f0', alpha=0.95)
    ax.text(0.02, 0.05, summary_text, transform=ax.transAxes, fontsize=10,
            verticalalignment='bottom', bbox=props, zorder=20, color='#1e293b')

    # Legend
    legend = ax.legend(loc='upper right', frameon=True, fontsize=9,
                       facecolor='white', edgecolor='#e2e8f0')
    legend.get_frame().set_alpha(0.8)

    plt.tight_layout()
    png_path = csv_path.with_name(csv_path.stem + '_smoothed.png')
    plt.savefig(png_path, dpi=300, bbox_inches='tight')
    plt.close()

    return png_path

def main():
    import sys

    if len(sys.argv) > 1:
        csv_path = Path(sys.argv[1])
    else:
        # Default to most recent CSV in script directory
        script_dir = Path(__file__).parent
        csvs = sorted(script_dir.glob('battery_discharge_*.csv'), reverse=True)
        if not csvs:
            print("No CSV files found. Usage: python regenerate_graph.py <csv_file>")
            return 1
        csv_path = csvs[0]

    print(f"Loading: {csv_path}")
    data = load_csv(csv_path)
    print(f"Loaded {len(data)} samples")

    print(f"Applying moving average filter (window={SMOOTHING_WINDOW})...")
    png_path = generate_graph(csv_path, data)
    print(f"Saved: {png_path}")

    return 0

if __name__ == "__main__":
    exit(main())
