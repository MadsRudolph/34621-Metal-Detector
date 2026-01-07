# Metal Detector Development Roadmap

## Project Overview

This roadmap outlines the development of a VLF (Very Low Frequency) metal detector for DTU course 34621. The detector uses phase-based metal classification to distinguish between ferrous metals (iron, steel) and non-ferrous metals (copper, aluminum, gold, silver).

### Design Goals
- **Runtime:** 100+ minutes on a single 9V battery
- **Detection method:** Phase shift analysis at 2kHz
- **Metal classification:** Ferrous (phase < 65°) vs Non-ferrous (phase > 65°)
- **User feedback:** OLED display + audio buzzer

### Your Current Setup
- **Test Equipment:** Oscilloscope + Signal Generator (allows full signal chain testing without coils)
- **ADC:** Internal 10-bit ADC on ATmega2560 (simpler than external MCP3208)
- **Coils:** Not built yet - signal generator will simulate RX signal for software development
- **TX Driver:** Direct MCU pin initially (add driver circuit later for actual coil drive)

---

## How The Detector Works

### Signal Flow
```
TX Coil (2kHz) → Magnetic Field → Target Metal → Eddy Currents →
→ Secondary Field → RX Coil → Amplifier → ADC → DFT → Phase/Magnitude → Display
```

### The Key Insight: Phase Detection
When the TX coil generates a 2kHz magnetic field:
- **Ferrous metals** (iron, steel) have high magnetic permeability - they "pull" the field, causing a small phase shift (< 65°)
- **Non-ferrous metals** (copper, coins) generate eddy currents that oppose the field, causing a larger phase shift (> 65°)

By measuring the phase difference between TX and RX signals, we can classify the metal type.

### Why 4× Oversampling?
We sample at 8kHz (4× the 2kHz TX frequency). This is a clever optimization:
- At exactly 4× oversampling, the DFT coefficients become trivial: {1, 0, -1, 0}
- No sine/cosine calculations needed - just additions and subtractions
- Dramatically reduces CPU load on the ATmega2560

---

## Current Codebase Status

### Fully Implemented (Ready to Use)
- [x] `tx.c/h` - 2kHz PWM generation on Timer1/OC1A (Pin 11)
- [x] `I2C.c/h` - Complete TWI (I2C) master driver for ATmega2560
- [x] `ssd1306.c/h` - Full OLED display driver with text, graphics, and fonts
- [x] `data.h` - Font data (8x8 ASCII + large numbers)

### Partially Implemented (Needs Completion)
- [ ] `rx.c/h` - ADC initialization exists but not integrated into main loop
- [ ] `dsp.c/h` - DFT accumulator algorithm works, but `DFT_Calc()` (magnitude/phase) is empty

### Not Yet Implemented (Stub Files Only)
- [ ] `detector.c/h` - Metal classification logic
- [ ] `display.c/h` - Screen rendering and UI
- [ ] `ui.c/h` - Button handling and buzzer control
- [ ] `config.h` - Pin definitions (currently just comments)
- [ ] `main.c` - Only calls `timer1_init()`, main loop is empty

---

## Development Phases

Each phase builds on the previous one. Complete all tasks and pass all tests before moving to the next phase.

---

## Phase 1: TX Signal Verification

### Goal
Verify that the existing TX code correctly generates a 2kHz square wave. This is the foundation of the entire detector - if TX isn't working, nothing else will.

### Why This Matters
The TX signal drives the transmit coil and serves as the reference for phase detection. It must be exactly 2kHz (±1%) for the DFT algorithm to work correctly.

### Files Involved
- `Code/src/main.c` - Entry point, calls `timer1_init()`
- `Code/src/signal/tx.c` - Timer1 configuration for 2kHz PWM
- `Code/src/signal/tx.h` - Function prototype

### Technical Details
Timer1 is configured in CTC (Clear Timer on Compare) mode:
- Clock: 16MHz (no prescaler)
- Compare value: 4000 (16MHz / 4000 = 4kHz toggle rate = 2kHz square wave)
- Output: OC1A (Pin 11 on Arduino Mega)

### Hardware Setup
1. Connect oscilloscope probe to Pin 11 (OC1A)
2. Connect scope ground to Arduino GND
3. Set scope to: 500µs/div horizontal, 2V/div vertical, DC coupling

### Tasks
- [x] Upload current code to Arduino Mega
- [x] Connect oscilloscope to Pin 11
- [x] Verify 2kHz square wave (period = 500µs)
- [x] Verify amplitude is 0-5V (full CMOS swing)
- [x] Verify duty cycle is 50%

### Expected Results
- Frequency: 2000 Hz ± 20 Hz
- Period: 500 µs
- High level: ~5V
- Low level: ~0V
- Duty cycle: 50%

### Troubleshooting
- **No signal:** Check that `timer1_init()` is called in main, verify Pin 11 connection
- **Wrong frequency:** Check OCR1A value in tx.c (should be 3999 for 2kHz)
- **Low amplitude:** Normal if measuring through voltage divider, should be 5V at pin

---

## Phase 2: Display Foundation

### Goal
Get the OLED display working so we have visual feedback for all future debugging. The display will show ADC values, DFT results, and detector status throughout development.

### Why This Matters
Without a display, debugging is extremely difficult. Serial output is possible but slow. Having real-time visual feedback on the OLED makes development much faster.

### Files Involved
- `Code/src/main.c` - Add display initialization
- `Code/src/app/display.c` - Create wrapper functions (currently stub)
- `Code/src/app/display.h` - Function prototypes (currently stub)
- `Code/src/drivers/I2C.c` - Already complete
- `Code/src/drivers/ssd1306.c` - Already complete

### Technical Details
The SSD1306 is a 128x64 pixel OLED display connected via I2C:
- I2C Address: 0x3C (7-bit) or 0x78 (8-bit with R/W bit)
- I2C Speed: 400kHz (Fast mode)
- SDA: Pin 20 on Arduino Mega
- SCL: Pin 21 on Arduino Mega

The existing `ssd1306.c` driver provides:
- `SSD1306_Init()` - Initialize display with correct command sequence
- `SSD1306_Clear()` - Clear display buffer
- `SSD1306_sendStr()` - Print string at current position
- `SSD1306_sendStrXY()` - Print string at specific X,Y position
- `SSD1306_SetXY()` - Set cursor position

### Hardware Setup
1. Connect SSD1306 VCC to Arduino 5V (or 3.3V if module has regulator)
2. Connect SSD1306 GND to Arduino GND
3. Connect SSD1306 SDA to Pin 20
4. Connect SSD1306 SCL to Pin 21

### Tasks
- [x] Wire OLED display to Arduino Mega (I2C pins 20, 21)
- [x] Add `I2C_Init()` call to main.c
- [x] Add `SSD1306_Init()` call to main.c
- [x] Add `SSD1306_Clear()` call to main.c
- [x] Display "Metal Detector" text on screen
- [x] Create `display_init()` wrapper in display.c
- [x] Create `display_clear()` wrapper in display.c
- [x] Create `display_text(x, y, string)` wrapper in display.c

### Expected Results
- Display shows "Metal Detector" text clearly
- No flickering or artifacts
- Text is readable and properly positioned

### Troubleshooting
- **Blank display:** Check I2C connections, verify I2C address (try 0x3D if 0x3C doesn't work)
- **Garbled display:** I2C speed may be too fast, check pull-up resistors (4.7kΩ on SDA/SCL)
- **Display works then freezes:** Check for I2C bus lockup, may need timeout handling

---

## Phase 3: ADC Input Chain

### Goal
Read analog values from the ADC and display them on the OLED. This establishes the input path for the RX signal.

### Why This Matters
The ADC converts the analog RX signal into digital samples for processing. We need to verify it's working correctly before adding timer-based sampling.

### Files Involved
- `Code/src/signal/rx.c` - ADC configuration and reading
- `Code/src/signal/rx.h` - Function prototypes
- `Code/src/config.h` - Pin definitions
- `Code/src/main.c` - Main loop with ADC reading

### Technical Details
The ATmega2560 has a 10-bit ADC with the following characteristics:
- Resolution: 10 bits (0-1023)
- Reference: AVCC (5V) by default
- Input range: 0V to VREF (5V)
- Channels: ADC0-ADC15 (we use ADC0 = Pin A0)
- Conversion time: 13-260 ADC clock cycles

ADC Configuration:
- ADMUX register: Select channel and reference voltage
- ADCSRA register: Enable ADC, set prescaler, start conversion
- ADCL/ADCH registers: Read result (low byte first)

### Hardware Setup
1. Connect signal generator to Pin A0 (through voltage divider if needed)
2. Set signal generator to DC mode
3. Vary output between 0V and 5V
4. **Important:** Never exceed 5V on ADC input!

### Tasks
- [x] Add pin definitions to `config.h` (ADC_PIN, etc.)
- [x] Implement `adc_init()` function in rx.c
- [x] Implement `adc_read()` function in rx.c (single conversion)
- [x] Call `adc_init()` from main.c
- [x] Read ADC value in main loop
- [x] Display ADC value on OLED (update every 100ms)
- [x] Test with signal generator DC output (0-5V range)

### Expected Results
- Display shows value 0-1023 corresponding to input voltage
- 0V input → display shows ~0
- 2.5V input → display shows ~512
- 5V input → display shows ~1023
- Value updates smoothly as voltage changes

### Troubleshooting
- **Always reads 0:** Check ADC is enabled (ADEN bit), check pin connection
- **Always reads 1023:** Input voltage too high, or floating input (add pull-down)
- **Noisy readings:** Add 100nF capacitor from A0 to GND, use averaging

---

## Phase 4: Timer-Based Sampling at 8kHz

### Goal
Sample the ADC at a precise 8kHz rate using Timer3 interrupts. Fill a 64-sample buffer for DFT processing.

### Why This Matters
Accurate timing is critical for phase detection. The DFT algorithm assumes exactly 4 samples per cycle of the 2kHz signal. Any timing error will cause phase measurement errors.

### Files Involved
- `Code/src/signal/rx.c` - Timer3 ISR and buffer management
- `Code/src/signal/rx.h` - Buffer declarations
- `Code/src/main.c` - Check for buffer complete flag

### Technical Details
Timer3 Configuration for 8kHz:
- Clock: 16MHz
- Prescaler: 8 (2MHz timer clock)
- Compare value: 250 (2MHz / 250 = 8kHz)
- Mode: CTC (Clear Timer on Compare)
- Interrupt: TIMER3_COMPA_vect

Sampling Strategy:
- Timer3 compare match triggers ADC conversion
- ADC complete interrupt stores sample in buffer
- After 64 samples, set flag and swap buffers (double buffering)

Buffer Structure:
```c
volatile uint16_t adc_buffer[64];  // Current buffer being filled
volatile uint8_t buffer_ready;     // Flag: buffer ready for processing
volatile uint8_t sample_index;     // Current position in buffer (0-63)
```

### Hardware Setup
1. Keep OLED connected for debug display
2. Connect signal generator to A0:
   - Waveform: Sine wave
   - Frequency: 2kHz
   - Amplitude: 2V peak-to-peak
   - DC Offset: 2.5V (so signal stays in 1.5V to 3.5V range)
3. Connect scope to a debug pin (e.g., Pin 13) to verify 8kHz timing

### Tasks
- [ ] Configure Timer3 for 8kHz interrupt
- [ ] Implement `timer3_init()` in rx.c
- [ ] Create 64-sample buffer array
- [ ] Implement Timer3 ISR to trigger ADC
- [ ] Implement ADC complete ISR to store samples
- [ ] Set buffer_ready flag after 64 samples
- [ ] Toggle debug pin in ISR for scope verification
- [ ] Display sample count or buffer status on OLED
- [ ] Verify 8kHz timing on oscilloscope

### Expected Results
- Debug pin toggles at exactly 8kHz (125µs period)
- Buffer fills with 64 samples every 8ms
- buffer_ready flag sets every 8ms
- No missed samples or timing jitter

### Troubleshooting
- **Wrong frequency:** Check Timer3 prescaler and compare value
- **Missed samples:** ADC conversion may be too slow, reduce prescaler
- **Buffer overflow:** Ensure buffer_ready is cleared after processing

---

## Phase 5: DFT Processing

### Goal
Compute magnitude and phase from the 64-sample buffer using the optimized single-bin DFT algorithm.

### Why This Matters
The DFT extracts the 2kHz component from the sampled signal. The magnitude tells us signal strength, and the phase tells us metal type.

### Files Involved
- `Code/src/signal/dsp.c` - DFT algorithm (partially implemented)
- `Code/src/signal/dsp.h` - Function prototypes and data types

### Technical Details

**Existing DFT_sum() Function:**
The `DFT_sum()` function is already implemented. It accumulates Re and Im components using the 4× oversampling trick:
- Sample 0: Re += sample (cos = 1)
- Sample 1: Im -= sample (sin = -1)
- Sample 2: Re -= sample (cos = -1)
- Sample 3: Im += sample (sin = 1)
- Repeat for all 64 samples

**Missing DFT_Calc() Function:**
After accumulation, we need to calculate:
- Magnitude: `sqrt(Re² + Im²)`
- Phase: `atan2(-Im, Re)` in degrees

**Integer Math Considerations:**
To avoid floating point on ATmega:
- Use integer square root approximation
- Use lookup table for atan2 (or CORDIC algorithm)
- Scale results appropriately

**Magnitude Calculation (Integer):**
```c
// Fast integer square root (Newton's method)
uint32_t isqrt(uint32_t n) {
    uint32_t x = n;
    uint32_t y = (x + 1) / 2;
    while (y < x) {
        x = y;
        y = (x + n/x) / 2;
    }
    return x;
}
```

**Phase Calculation:**
- Use atan2 lookup table (256 entries for 0-90°, mirror for other quadrants)
- Or use CORDIC algorithm for better accuracy
- Result in degrees (0-360°)

### Hardware Setup
Same as Phase 4:
- Signal generator: 2kHz sine, 2Vpp, 2.5V DC offset
- Vary amplitude to test magnitude response
- Vary phase (if your sig gen supports it) to test phase response

### Tasks
- [ ] Implement integer square root function
- [ ] Implement atan2 approximation (lookup table or CORDIC)
- [ ] Complete `DFT_Calc()` function
- [ ] Calculate magnitude from Re_buff and Im_buff
- [ ] Calculate phase in degrees (0-360°)
- [ ] Display magnitude and phase on OLED
- [ ] Test: 2kHz input should show high magnitude
- [ ] Test: 1kHz or 3kHz input should show low magnitude (out of band)
- [ ] Test: Varying amplitude should change magnitude proportionally
- [ ] Test: Phase reading should be stable (±5° when signal is steady)

### Expected Results
- 2kHz sine input: High magnitude, stable phase
- Off-frequency input: Low magnitude (DFT rejects other frequencies)
- Phase varies smoothly with input phase shift
- No overflow in calculations (check 32-bit math)

### Troubleshooting
- **Magnitude always zero:** Check DFT_sum is being called, check buffer is valid
- **Phase jumps around:** Magnitude may be too low (noise floor), add threshold
- **Overflow errors:** Use 32-bit integers for accumulator, check scaling

---

## Phase 6: Detector Logic

### Goal
Implement metal classification based on phase angle. Add calibration to establish a baseline.

### Why This Matters
This is the core detection algorithm. It converts raw phase measurements into meaningful "FERROUS" or "NON-FERROUS" classifications.

### Files Involved
- `Code/src/app/detector.c` - Detection algorithm
- `Code/src/app/detector.h` - Function prototypes and types

### Technical Details

**Phase Threshold:**
- Phase < 65°: Ferrous metal (iron, steel)
- Phase > 65°: Non-ferrous metal (copper, aluminum, gold)
- The 65° threshold is empirical - may need adjustment for your coils

**Calibration:**
Before detection, calibrate with no metal present:
1. Collect N samples (e.g., 32) of phase readings
2. Calculate average phase (baseline)
3. Store baseline in memory
4. During detection, compare measured phase to baseline

**Detection Algorithm:**
```c
typedef enum {
    METAL_NONE,
    METAL_FERROUS,
    METAL_NONFERROUS
} metal_type_t;

metal_type_t detect_metal(int16_t phase, int16_t baseline) {
    int16_t deviation = phase - baseline;

    if (magnitude < DETECTION_THRESHOLD) {
        return METAL_NONE;  // No significant signal
    }

    if (phase < 65) {
        return METAL_FERROUS;
    } else {
        return METAL_NONFERROUS;
    }
}
```

**Signal Strength Indicator:**
Use magnitude to indicate how strong the detection is. Higher magnitude = closer to coil or larger object.

### Hardware Setup
For now, simulate with signal generator:
- Set phase < 65° to simulate ferrous metal
- Set phase > 65° to simulate non-ferrous metal
- Vary magnitude to test detection threshold

### Tasks
- [ ] Define metal_type_t enum (NONE, FERROUS, NONFERROUS)
- [ ] Implement `detector_init()` function
- [ ] Implement `detector_calibrate()` function
- [ ] Implement `detector_get_metal_type()` function
- [ ] Add magnitude threshold for valid detection
- [ ] Store calibration baseline in global variable
- [ ] Display metal type on OLED ("FERROUS", "NON-FERROUS", "---")
- [ ] Display signal strength indicator (bar graph or percentage)
- [ ] Test with simulated phase values from signal generator

### Expected Results
- Calibration stores baseline phase
- Phase < 65° shows "FERROUS"
- Phase > 65° shows "NON-FERROUS"
- Low magnitude shows "---" (no detection)
- Signal strength corresponds to magnitude

### Troubleshooting
- **Always shows same type:** Check phase calculation is correct
- **Erratic detection:** Lower magnitude threshold, add hysteresis
- **Calibration doesn't work:** Ensure enough samples, check averaging

---

## Phase 7: User Interface

### Goal
Add physical buttons for Start/Stop and Calibrate, plus a buzzer for audio feedback.

### Why This Matters
A metal detector needs hands-free operation. Audio feedback lets the user focus on sweeping the coil rather than watching the display.

### Files Involved
- `Code/src/app/ui.c` - Button and buzzer handling
- `Code/src/app/ui.h` - Function prototypes
- `Code/src/config.h` - Pin definitions

### Technical Details

**Button Configuration:**
- Start/Stop: Pin 2 (PE4/INT4) - External interrupt
- Calibrate: Pin 3 (PE5/INT5) - External interrupt
- Internal pull-ups enabled, buttons connect to GND
- Debounce: 50ms delay after edge detection

**External Interrupt Setup:**
```c
// Enable INT4 and INT5
EIMSK |= (1 << INT4) | (1 << INT5);
// Falling edge trigger (button press pulls low)
EICRB |= (1 << ISC41) | (1 << ISC51);
```

**Buzzer Configuration:**
- Pin 8 (PH5/OC4C) - Timer4 PWM output
- Frequency: Variable (200Hz - 2kHz based on signal strength)
- Duty cycle: 50% for loudest tone

**Beep Rate Algorithm:**
```c
// Beep faster as signal gets stronger
uint16_t beep_interval = 1000 - (magnitude * 900 / MAX_MAGNITUDE);
// Range: 1000ms (weak) to 100ms (strong)
```

### Hardware Setup
1. Connect momentary pushbutton between Pin 2 and GND (Start/Stop)
2. Connect momentary pushbutton between Pin 3 and GND (Calibrate)
3. Connect piezo buzzer between Pin 8 and GND (through 100Ω resistor)

### Tasks
- [ ] Add button pin definitions to config.h
- [ ] Add buzzer pin definition to config.h
- [ ] Implement `ui_init()` - configure pins and interrupts
- [ ] Implement INT4 ISR for Start/Stop button
- [ ] Implement INT5 ISR for Calibrate button
- [ ] Add debounce logic (50ms lockout after press)
- [ ] Configure Timer4 for buzzer PWM
- [ ] Implement `buzzer_on(frequency)` function
- [ ] Implement `buzzer_off()` function
- [ ] Implement `buzzer_beep(duration)` function
- [ ] Variable beep rate based on signal strength
- [ ] Test: Start button enables detection
- [ ] Test: Stop button pauses detection
- [ ] Test: Calibrate button triggers calibration
- [ ] Test: Buzzer beeps faster with stronger signal

### Expected Results
- Buttons respond to presses with no bouncing
- Start/Stop toggles detector state
- Calibrate initiates calibration sequence
- Buzzer produces clean tones at various frequencies
- Beep rate increases with signal strength

### Troubleshooting
- **Button triggers multiple times:** Increase debounce delay
- **Button doesn't respond:** Check interrupt enable bits, check pin connection
- **Buzzer too quiet:** Increase duty cycle, use amplifier, check resistor value
- **Buzzer whines constantly:** Check PWM frequency is in audible range

---

## Phase 8: Display Screens

### Goal
Create polished display screens for different detector states.

### Why This Matters
Good UI makes the detector easy to use. Clear screens for each state help the user understand what's happening.

### Files Involved
- `Code/src/app/display.c` - Screen rendering functions
- `Code/src/app/display.h` - Function prototypes

### Screen Designs

**Splash Screen (Startup):**
```
+------------------+
|                  |
|  METAL DETECTOR  |
|                  |
|   DTU 34621      |
|                  |
|  Initializing... |
+------------------+
```

**Detection Screen (Running):**
```
+------------------+
| Signal: ████████ |
| Phase:  45.2°    |
|                  |
|    FERROUS       |
|     (Iron)       |
|                  |
+------------------+
```

**Calibration Screen:**
```
+------------------+
|                  |
|  CALIBRATING...  |
|                  |
|  Keep coil away  |
|  from metal      |
|                  |
|  [████░░░░] 50%  |
+------------------+
```

**Stopped Screen:**
```
+------------------+
|                  |
|     STOPPED      |
|                  |
|  Press START to  |
|  begin detection |
|                  |
+------------------+
```

### Tasks
- [ ] Implement `display_splash_screen()` - shown at startup
- [ ] Implement `display_detection_screen(magnitude, phase, metal_type)`
- [ ] Implement `display_calibration_screen(progress)` - progress bar
- [ ] Implement `display_stopped_screen()`
- [ ] Create signal strength bar graph function
- [ ] Create large text function for metal type
- [ ] Add screen transition logic
- [ ] Test all screens visually

### Expected Results
- All screens render correctly without glitches
- Text is readable and well-positioned
- Bar graph updates smoothly
- Screen transitions are clean (no flicker)

---

## Phase 9: Integration and Main Loop

### Goal
Combine all components into a working system with proper state machine.

### Why This Matters
This brings everything together into a usable metal detector.

### Files Involved
- `Code/src/main.c` - Main loop and state machine

### State Machine

```
                    ┌─────────────┐
                    │   STARTUP   │
                    └──────┬──────┘
                           │ (init complete)
                           ▼
    ┌──────────────────────────────────────┐
    │                                      │
    │              ┌─────────────┐         │
    │   ┌─────────►│    IDLE     │◄────┐   │
    │   │          └──────┬──────┘     │   │
    │   │                 │ (START)    │   │
    │   │ (STOP)          ▼            │   │
    │   │          ┌─────────────┐     │   │
    │   └──────────│   RUNNING   │─────┘   │
    │              └──────┬──────┘         │
    │                     │ (CALIBRATE)    │
    │                     ▼                │
    │              ┌─────────────┐         │
    │              │ CALIBRATING │─────────┘
    │              └─────────────┘ (done)
    │                                      │
    └──────────────────────────────────────┘
```

### Main Loop Structure

```c
int main(void) {
    // Initialize all subsystems
    system_init();

    // Show splash screen
    display_splash_screen();
    _delay_ms(2000);

    // Enter idle state
    state = STATE_IDLE;
    display_stopped_screen();

    while (1) {
        switch (state) {
            case STATE_IDLE:
                // Wait for START button
                // Low power mode possible here
                break;

            case STATE_RUNNING:
                if (buffer_ready) {
                    buffer_ready = 0;
                    DFT_Calc();
                    metal_type = detect_metal(phase, baseline);
                    display_detection_screen(magnitude, phase, metal_type);
                    update_buzzer(magnitude);
                }
                break;

            case STATE_CALIBRATING:
                run_calibration();
                state = STATE_RUNNING;
                break;
        }
    }
}
```

### Power Optimization
- Limit display updates to 12Hz (every 83ms)
- Only update changed portions of display
- Disable unused peripherals
- Target: < 120mA total system current

### Tasks
- [ ] Create state enum (STARTUP, IDLE, RUNNING, CALIBRATING)
- [ ] Implement `system_init()` to call all init functions
- [ ] Implement state machine in main loop
- [ ] Connect button interrupts to state changes
- [ ] Rate-limit display updates (12Hz max)
- [ ] Integrate all subsystems:
  - [ ] TX generation (already running)
  - [ ] ADC sampling with Timer3
  - [ ] DFT processing
  - [ ] Metal detection
  - [ ] Display updates
  - [ ] Buzzer feedback
- [ ] Test complete system with signal generator
- [ ] Verify all state transitions work correctly

### Expected Results
- Detector starts in IDLE state
- START button → RUNNING state, detection active
- STOP button → IDLE state, detection paused
- CALIBRATE button → CALIBRATING state, then back to RUNNING
- Display updates at steady 12Hz
- Buzzer provides appropriate feedback

---

## Phase 10: TX Driver and Coils (Hardware Build)

### Goal
Build the physical TX driver circuit and wind the coils for real metal detection.

### Status
**Deferred** until all software is working with signal generator simulation.

### TX Driver Options

**Option A: Simple NPN Switch (Recommended for Starting)**
```
Components:
- 2N2222A NPN transistor
- 1.5kΩ base resistor
- 1N4148 flyback diode

Circuit:
Pin 11 ──[1.5kΩ]──┬── Base
                  │
            Collector ──┬── TX Coil ──┬── +9V
                       │             │
                 [1N4148]            │
                       │             │
               Emitter ┴─────────────┴── GND

Specs:
- Efficiency: 94%
- Drive current: 2.9mA from MCU
- Coil current: ~40mA
```

**Option B: MOSFET Half-Bridge (Best Efficiency)**
```
Components:
- Si2301 P-channel MOSFET (high side)
- Si2302 N-channel MOSFET (low side)
- 2× 100Ω gate resistors

Specs:
- Efficiency: 99.9%
- Drive current: ~0mA (capacitive gates)
- Coil current: ~40mA
- Requires dead-time control to prevent shoot-through
```

### Coil Specifications (Concentric Design - Recommended)

> **Important:** Use **0.52mm wire (AWG 24)** instead of 0.3mm for much better efficiency!

**TX Coil:**
| Parameter | Value | Notes |
|-----------|-------|-------|
| Inductance | **15 mH** | Target value |
| Diameter | 150 mm | Outer coil |
| Turns | ~130 | |
| Wire | **0.52 mm** (AWG 24) | Thicker = lower R |
| DC Resistance | **4.7 Ω** | vs 15Ω with 0.3mm |
| Wire length | ~59 m | |
| Impedance @ 2kHz | **189 Ω** | |
| Q Factor | **40** | High Q = efficient |
| Current @ 7.5V | **39.7 mA** | Within budget ✓ |

**RX Coil:**
| Parameter | Value |
|-----------|-------|
| Diameter | 60 mm (inner) |
| Turns | 180 |
| Inductance | 12 mH |

**Bucking Coil:**
| Parameter | Value |
|-----------|-------|
| Diameter | 80 mm (middle) |
| Turns | 15-25 (adjust for null) |

> **Why 0.52mm wire?** 3× higher Q factor, 67% less power wasted as heat

### Tasks
- [ ] Choose TX driver topology (NPN or MOSFET)
- [ ] Gather components for TX driver
- [ ] Build TX driver on breadboard
- [ ] Test TX driver with oscilloscope (verify switching)
- [ ] Measure TX driver current (target: 35-40mA)
- [ ] Wind TX coil (150mm diameter, ~130 turns)
- [ ] Measure coil inductance (target: 15mH)
- [ ] Wind RX coil (same specs, or concentric design)
- [ ] Build RX amplifier circuit (gain ~100×)
- [ ] Connect TX and RX coils to system
- [ ] Test with real metal objects

### Expected Results
- TX driver switches cleanly at 2kHz
- Coil current is 35-40mA
- Coil inductance is approximately 15mH
- System detects metal objects at 50mm+ range
- Ferrous and non-ferrous metals correctly classified

---

## Hardware Pin Assignments

| Pin | Name | Function | Notes |
|-----|------|----------|-------|
| 11 | PB5/OC1A | TX PWM Output | 2kHz square wave to driver |
| 8 | PH5/OC4C | Buzzer PWM | Variable frequency audio |
| 2 | PE4/INT4 | Start/Stop Button | External interrupt, active low |
| 3 | PE5/INT5 | Calibrate Button | External interrupt, active low |
| 20 | PD1/SDA | I2C Data | OLED display |
| 21 | PD0/SCL | I2C Clock | OLED display |
| A0 | PF0/ADC0 | RX Signal Input | Analog input 0-5V |
| 13 | PB7 | Debug Output | Optional: timing verification |

---

## Key System Parameters

| Parameter | Value | Notes |
|-----------|-------|-------|
| TX Frequency | 2000 Hz | Timer1 CTC mode |
| Sample Rate | 8000 Hz | Timer3 CTC mode (4× TX) |
| DFT Window | 64 samples | 8ms processing window |
| Phase Threshold | 65° | Ferrous < 65° < Non-ferrous |
| Display Refresh | 12 Hz | 83ms update interval |
| Max System Current | 120 mA | Battery life constraint |
| Target Runtime | 100 min | On 9V alkaline battery |

---

## Operating Modes

The detector can run in two modes. Choose based on your priority:

### Maximum Power Mode (Recommended)
> **Use when:** Detection depth is priority, runtime exactly meets 100 min requirement.

| Parameter | Value |
|-----------|-------|
| TX Duty Cycle | 100% (continuous) |
| TX Current | 40 mA |
| Total System Current | 120 mA |
| Expected Runtime | ~100 min |
| Detection Performance | **Maximum** |

```c
// No duty cycling needed - continuous operation
while (1) {
    if (buffer_ready) {
        compute_dft();
        update_display();
    }
}
```

### Conservative Mode (20% safety margin)
> **Use when:** Want runtime safety margin, accept slightly reduced detection depth.

| Parameter | Value |
|-----------|-------|
| TX Duty Cycle | 80% (160ms ON / 40ms OFF) |
| TX Current (average) | 32 mA |
| Total System Current | ~100 mA |
| Expected Runtime | ~120 min |
| Detection Performance | 80% field, **93% depth** (cube-root relationship) |

```c
#define ACTIVE_PERIOD_MS   160   // 320 TX cycles at 2kHz
#define SLEEP_PERIOD_MS    40    // Brief sleep

void measurement_cycle(void) {
    enable_tx_driver();
    _delay_ms(5);  // Coil stabilization
    sample_for_ms(155);
    disable_tx_driver();
    compute_dft();
    update_display();
    sleep_for_ms(30);
}
```

### Power Optimization Tips

| Optimization | Savings | Impact |
|--------------|---------|--------|
| **Remove Power LED** | 8 mA | Zero functional impact, +10 min runtime |
| Reduce OLED refresh (12Hz → 6Hz) | 3-5 mA | Slightly slower display updates |
| Use MOSFET driver (vs NPN) | 3 mA | Better efficiency |

> **Recommendation:** Remove Power LED (cut PWR jumper or desolder) → gives 10 min safety margin at maximum power.

---

## File Structure Reference

```
Code/src/
├── main.c                 # Entry point and main loop
├── config.h               # Pin definitions and constants
│
├── drivers/               # Hardware abstraction layer
│   ├── I2C.c/h           # I2C (TWI) master driver [DONE]
│   ├── ssd1306.c/h       # OLED display driver [DONE]
│   └── data.h            # Font and graphics data [DONE]
│
├── signal/                # Signal processing layer
│   ├── tx.c/h            # TX signal generation [DONE]
│   ├── rx.c/h            # RX signal sampling [PARTIAL]
│   └── dsp.c/h           # DFT processing [PARTIAL]
│
└── app/                   # Application layer
    ├── detector.c/h      # Metal detection logic [TODO]
    ├── display.c/h       # Screen rendering [TODO]
    └── ui.c/h            # Buttons and buzzer [TODO]
```

---

## Progress Checklist

### Phase 1: TX Verification
- [ ] Upload code and verify 2kHz on scope

### Phase 2: Display Foundation
- [ ] OLED wired and showing text

### Phase 3: ADC Input Chain
- [ ] ADC reading displayed on OLED

### Phase 4: 8kHz Sampling
- [ ] Timer3 ISR running at 8kHz
- [ ] 64-sample buffer filling correctly

### Phase 5: DFT Processing
- [ ] Magnitude calculation working
- [ ] Phase calculation working

### Phase 6: Detector Logic
- [ ] Calibration implemented
- [ ] Metal classification working

### Phase 7: User Interface
- [ ] Start/Stop button working
- [ ] Calibrate button working
- [ ] Buzzer feedback working

### Phase 8: Display Screens
- [ ] All screens implemented

### Phase 9: Integration
- [ ] State machine working
- [ ] Full system test passed

### Phase 10: Hardware Build
- [ ] TX driver built and tested
- [ ] Coils wound and measured
- [ ] Real metal detection working
