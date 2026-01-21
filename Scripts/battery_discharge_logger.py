#!/usr/bin/env python3
"""
Battery Discharge Logger for Analog Discovery 3
Uses the Digilent WaveForms SDK (dwf) via ctypes.
"""

import ctypes
import sys
import time
import signal
import csv
from datetime import datetime
from pathlib import Path

# =============================================================================
# Test Configuration
# =============================================================================
TEST_NAME = "Metal Detector Battery Discharge Test"
SAMPLE_INTERVAL_SEC = 1.0
DISPLAY_INTERVAL_SEC = 10.0
VOLTAGE_THRESHOLD_V = 6.0
MAX_RUNTIME_MIN = 180
PASS_RUNTIME_MIN = 100

# =============================================================================
# WaveForms SDK Constants
# =============================================================================
enumfilterAll = 0
acqmodeSingle = 0
trigsrcNone = 0
DwfStateReady = 2

# =============================================================================
# Load WaveForms SDK
# =============================================================================
def load_dwf_library():
    if sys.platform.startswith('win'):
        paths = [
            "dwf.dll",
            r"C:\Program Files (x86)\Digilent\WaveFormsSDK\dwf.dll",
            r"C:\Program Files\Digilent\WaveFormsSDK\dwf.dll",
        ]
        for path in paths:
            try:
                return ctypes.cdll.LoadLibrary(path)
            except OSError:
                continue
        raise OSError("Could not find dwf.dll")
    elif sys.platform.startswith('linux'):
        return ctypes.cdll.LoadLibrary("libdwf.so")
    elif sys.platform.startswith('darwin'):
        return ctypes.cdll.LoadLibrary("/Library/Frameworks/dwf.framework/dwf")
    raise OSError(f"Unsupported platform: {sys.platform}")

# =============================================================================
# Global state
# =============================================================================
running = True

def signal_handler(signum, frame):
    global running
    print("\n\nStopping measurement...")
    running = False

# =============================================================================
# AD3 Interface
# =============================================================================
class AnalogDiscovery3:
    def __init__(self, dwf_lib):
        self.dwf = dwf_lib
        self.hdwf = ctypes.c_int()
        self.connected = False

    def connect(self):
        device_count = ctypes.c_int()
        self.dwf.FDwfEnum(enumfilterAll, ctypes.byref(device_count))
        if device_count.value == 0:
            raise RuntimeError("No Analog Discovery device found")

        print(f"Found {device_count.value} device(s)")
        device_name = ctypes.create_string_buffer(64)
        self.dwf.FDwfEnumDeviceName(0, device_name)
        print(f"Connecting to: {device_name.value.decode()}")

        if self.dwf.FDwfDeviceOpen(-1, ctypes.byref(self.hdwf)) == 0:
            szerr = ctypes.create_string_buffer(512)
            self.dwf.FDwfGetLastErrorMsg(szerr)
            raise RuntimeError(f"Failed to open device: {szerr.value.decode()}")

        self.connected = True
        print("Device connected")
        self._configure()

    def _configure(self):
        self.dwf.FDwfAnalogInChannelEnableSet(self.hdwf, ctypes.c_int(0), ctypes.c_int(1))
        self.dwf.FDwfAnalogInChannelRangeSet(self.hdwf, ctypes.c_int(0), ctypes.c_double(25.0))
        self.dwf.FDwfAnalogInChannelOffsetSet(self.hdwf, ctypes.c_int(0), ctypes.c_double(0.0))
        self.dwf.FDwfAnalogInAcquisitionModeSet(self.hdwf, acqmodeSingle)
        self.dwf.FDwfAnalogInBufferSizeSet(self.hdwf, ctypes.c_int(1))
        self.dwf.FDwfAnalogInFrequencySet(self.hdwf, ctypes.c_double(1000.0))
        self.dwf.FDwfAnalogInTriggerSourceSet(self.hdwf, trigsrcNone)
        time.sleep(0.1)
        print("Analog input configured: Channel 1, +/-25V range")

    def read_voltage(self):
        self.dwf.FDwfAnalogInConfigure(self.hdwf, ctypes.c_int(0), ctypes.c_int(1))
        sts = ctypes.c_int()
        for _ in range(50):
            self.dwf.FDwfAnalogInStatus(self.hdwf, ctypes.c_int(1), ctypes.byref(sts))
            if sts.value == DwfStateReady:
                break
            time.sleep(0.01)
        voltage = ctypes.c_double()
        self.dwf.FDwfAnalogInStatusSample(self.hdwf, ctypes.c_int(0), ctypes.byref(voltage))
        return voltage.value

    def disconnect(self):
        if self.connected:
            self.dwf.FDwfDeviceClose(self.hdwf)
            self.connected = False
            print("Device disconnected")

# =============================================================================
# Graph Generation (separate function with its own import)
# =============================================================================
def generate_graph(csv_path, data, stop_reason):
    """Generate professional, publication-quality graph from collected data."""
    print("\nGenerating graph...")

    try:
        import matplotlib
        matplotlib.use('Agg')
        import matplotlib.pyplot as plt
        from matplotlib.patches import Rectangle
        import matplotlib.ticker as ticker
    except Exception as e:
        print(f"Could not load matplotlib: {e}")
        print("Run 'python -m pip install matplotlib' to enable graphs")
        return None

    if len(data) < 2:
        print("Not enough data for graph")
        return None

    times_min = [d['elapsed_minutes'] for d in data]
    voltages = [d['voltage_V'] for d in data]

    start_v = voltages[0]
    end_v = voltages[-1]
    total_runtime = times_min[-1]
    avg_rate = ((start_v - end_v) * 1000) / total_runtime if total_runtime > 0 else 0
    passed = total_runtime >= PASS_RUNTIME_MIN
    result = "PASS" if passed else "FAIL"

    # Styling Constants
    COLOR_PRIMARY = '#2563eb'  # Professional Blue
    COLOR_THRESHOLD = '#dc2626' # Red
    COLOR_PASS = '#16a34a'      # Green
    COLOR_BG = '#f8fafc'        # Very light grey/blue background
    COLOR_GRID = '#e2e8f0'      # Subtle grid
    COLOR_FAIL_ZONE = '#fee2e2' # Light red for fail region
    COLOR_PASS_ZONE = '#dcfce7' # Light green for pass region
    
    # Create Figure
    plt.style.use('default') # Reset to default first
    fig, ax = plt.subplots(figsize=(12, 7.5), dpi=150)
    fig.patch.set_facecolor('white')
    ax.set_facecolor(COLOR_BG)

    # 1. Background Zones (PASS/FAIL)
    max_x = max(total_runtime * 1.1, PASS_RUNTIME_MIN * 1.2, 1)
    ax.axvspan(0, PASS_RUNTIME_MIN, color=COLOR_FAIL_ZONE, alpha=0.3, label='Fail Zone')
    ax.axvspan(PASS_RUNTIME_MIN, max_x, color=COLOR_PASS_ZONE, alpha=0.3, label='Pass Zone')

    # 2. Main Discharge Curve with Gradient-like Fill
    ax.fill_between(times_min, voltages, 0, color=COLOR_PRIMARY, alpha=0.1)
    ax.plot(times_min, voltages, color=COLOR_PRIMARY, linewidth=2.5, 
            label='Battery Voltage', zorder=10)

    # 3. Threshold and Pass Criteria Lines
    ax.axhline(y=VOLTAGE_THRESHOLD_V, color=COLOR_THRESHOLD, linestyle='--', 
               linewidth=1.5, alpha=0.8, zorder=5, label=f'Threshold ({VOLTAGE_THRESHOLD_V}V)')
    ax.axvline(x=PASS_RUNTIME_MIN, color=COLOR_PASS, linestyle=(0, (5, 5)), 
               linewidth=1.5, alpha=0.8, zorder=5)

    # 4. Markers for Start and End
    ax.scatter(times_min[0], voltages[0], color=COLOR_PASS, edgecolors='white', 
               s=120, zorder=15, marker='o', linewidths=2, label=f'Start: {start_v:.2f}V')
    ax.scatter(times_min[-1], voltages[-1], color=COLOR_THRESHOLD, edgecolors='white', 
               s=120, zorder=15, marker='s', linewidths=2, label=f'End: {end_v:.2f}V')

    # 5. Professional Axis and Labels
    ax.set_xlabel('Time (minutes)', fontsize=11, fontweight='500', labelpad=10)
    ax.set_ylabel('Voltage (V)', fontsize=11, fontweight='500', labelpad=10)
    
    # Dynamic Title with cleaner look
    title_obj = ax.set_title(f'{TEST_NAME}', fontsize=16, fontweight='bold', pad=25, loc='left')
    plt.text(0, 1.02, f"Source: {stop_reason}", transform=ax.transAxes, 
             fontsize=10, color='#64748b', ha='left')

    # 6. Grid and Spines
    ax.grid(True, axis='y', linestyle='-', color=COLOR_GRID, alpha=0.7, zorder=0)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_color('#cbd5e1')
    ax.spines['bottom'].set_color('#cbd5e1')

    # 7. Axes Limits and Ticks
    ax.set_xlim(0, max_x)
    y_min = min(voltages) - 0.2
    y_max = max(voltages) + 0.2
    ax.set_ylim(max(0, y_min), y_max)
    ax.xaxis.set_major_locator(ticker.AutoLocator())
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter('%.2f V'))

    # 8. Modern Summary Box
    summary_text = (
        fr"$\mathbf{{SUMMARY}}$" "\n"
        f"Runtime: {total_runtime:.1f} min\n"
        f"Voltage Drop: {(start_v-end_v)*1000:.0f} mV\n"
        f"Avg Rate: {avg_rate:.1f} mV/min\n"
        fr"Status: $\mathbf{{{result}}}$"
    )
    
    props = dict(boxstyle='round,pad=0.8', facecolor='white', edgecolor='#e2e8f0', alpha=0.95)
    ax.text(0.02, 0.05, summary_text, transform=ax.transAxes, fontsize=10,
            verticalalignment='bottom', bbox=props, zorder=20,
            color='#1e293b')

    # 9. Clean Legend
    legend = ax.legend(loc='upper right', frameon=True, fontsize=9, 
                       facecolor='white', edgecolor='#e2e8f0')
    legend.get_frame().set_alpha(0.8)

    plt.tight_layout()
    png_path = csv_path.with_suffix('.png')
    plt.savefig(png_path, dpi=300, bbox_inches='tight')
    plt.close()

    print(f"Graph saved: {png_path}")
    return png_path

# =============================================================================
# Main
# =============================================================================
def main():
    global running

    print("=" * 60)
    print("BATTERY DISCHARGE LOGGER")
    print(f"Test: {TEST_NAME}")
    print("=" * 60)

    signal.signal(signal.SIGINT, signal_handler)

    # Load SDK
    print("\nLoading WaveForms SDK...")
    try:
        dwf = load_dwf_library()
        print("SDK loaded")
    except OSError as e:
        print(f"ERROR: {e}")
        return 1

    ad3 = AnalogDiscovery3(dwf)

    # Data storage
    data = []
    csv_path = None
    stop_reason = "Unknown"

    try:
        ad3.connect()

        initial_v = ad3.read_voltage()
        print(f"\nInitial voltage: {initial_v:.3f} V")

        if initial_v < VOLTAGE_THRESHOLD_V:
            print(f"ERROR: Voltage already below threshold ({VOLTAGE_THRESHOLD_V}V)")
            return 1

        # Create CSV
        start_time = datetime.now()
        csv_path = Path(__file__).parent / f"battery_discharge_{start_time.strftime('%Y%m%d_%H%M%S')}.csv"

        with open(csv_path, 'w', newline='') as f:
            f.write(f"# Test: {TEST_NAME}\n")
            f.write(f"# Date: {start_time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"# Threshold: {VOLTAGE_THRESHOLD_V}V\n")
            f.write(f"# Pass criteria: > {PASS_RUNTIME_MIN} min\n")
            f.write("#\n")
            writer = csv.writer(f)
            writer.writerow(['elapsed_seconds', 'elapsed_minutes', 'voltage_V'])

        print(f"Logging to: {csv_path}")
        print(f"\nPress Ctrl+C to stop\n")
        print("-" * 60)

        # Measurement loop
        t_start = time.time()
        last_display = 0
        sample_count = 0

        while running:
            elapsed_sec = time.time() - t_start
            elapsed_min = elapsed_sec / 60.0

            if elapsed_min >= MAX_RUNTIME_MIN:
                stop_reason = f"Max runtime ({MAX_RUNTIME_MIN} min)"
                break

            voltage = ad3.read_voltage()
            sample_count += 1

            # Store data
            sample = {'elapsed_seconds': elapsed_sec, 'elapsed_minutes': elapsed_min, 'voltage_V': voltage}
            data.append(sample)

            # Write to CSV
            with open(csv_path, 'a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([f"{elapsed_sec:.1f}", f"{elapsed_min:.3f}", f"{voltage:.4f}"])

            if voltage < VOLTAGE_THRESHOLD_V:
                stop_reason = f"Voltage threshold ({VOLTAGE_THRESHOLD_V}V)"
                break

            # Display update
            if elapsed_sec - last_display >= DISPLAY_INTERVAL_SEC:
                rate = 0
                if len(data) > 60:
                    dv = data[-60]['voltage_V'] - voltage
                    dt = elapsed_min - data[-60]['elapsed_minutes']
                    rate = (dv * 1000) / dt if dt > 0 else 0
                print(f"[{elapsed_min:6.1f} min] {voltage:.4f} V | {rate:6.2f} mV/min | {sample_count} samples")
                last_display = elapsed_sec

            # Wait for next sample
            next_time = t_start + (sample_count * SAMPLE_INTERVAL_SEC)
            sleep_time = next_time - time.time()
            if sleep_time > 0:
                time.sleep(sleep_time)

        if not running:
            stop_reason = "User stopped (Ctrl+C)"

    except Exception as e:
        print(f"\nERROR: {e}")
        stop_reason = f"Error: {e}"
        return 1

    finally:
        print("-" * 60)
        ad3.disconnect()

        # Print summary
        if data:
            total_min = data[-1]['elapsed_minutes']
            start_v = data[0]['voltage_V']
            end_v = data[-1]['voltage_V']
            drop_mv = (start_v - end_v) * 1000
            rate = drop_mv / total_min if total_min > 0 else 0
            passed = total_min >= PASS_RUNTIME_MIN

            print(f"""
{'='*60}
SUMMARY
{'='*60}
Stop reason:    {stop_reason}
Runtime:        {total_min:.1f} minutes
Start voltage:  {start_v:.3f} V
End voltage:    {end_v:.3f} V
Total drop:     {drop_mv:.1f} mV
Avg rate:       {rate:.2f} mV/min
{'='*60}
Result:         {'PASS' if passed else 'FAIL'} (need > {PASS_RUNTIME_MIN} min)
{'='*60}
CSV saved:      {csv_path}
""")

            # Generate graph
            if csv_path:
                generate_graph(csv_path, data, stop_reason)

    return 0

if __name__ == "__main__":
    result = main()
    print("\nPress Enter to close...")
    input()
    sys.exit(result)
