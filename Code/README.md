# VLF Metal Detector Firmware

Arduino Mega 2560 (ATmega2560) firmware for VLF metal detection.

## Structure

```
src/
├── config.h          # Pin definitions, constants
├── main.c            # Application entry, state machine
├── signal/
│   ├── adc.c/h       # 8kHz sampling, PWM, double buffer
│   └── dsp.c/h       # DFT analysis, IIR filter
├── app/
│   ├── detector.c/h  # Metal classification, calibration
│   ├── display.c/h   # OLED screens
│   └── ui.c/h        # Buttons, buzzer
└── drivers/          # Given drivers (do not modify)
    ├── I2C.c/h
    ├── ssd1306_driver.c/h
    └── data.h
```

## Build

```bash
pio run              # Build
pio run -t upload    # Upload
pio device monitor   # Serial monitor
```

## Pins

| Pin | Function |
|-----|----------|
| 9   | PWM to TX coil |
| 10  | MCP3208 CS |
| 50  | SPI MISO |
| 51  | SPI MOSI |
| 52  | SPI SCK |
| 8   | Buzzer |
| 2   | Start/Stop (INT4) |
| 3   | Calibrate (INT5) |
| 20  | I2C SDA |
| 21  | I2C SCL |

## Signal Flow

```
PWM (2kHz) --> TX Coil --> [Metal] --> RX Coil --> MCP3208 ADC
                                                        |
Timer1 ISR (8kHz) ---> Double Buffer ---> Main Loop
                                              |
                            DFT --> Filter --> Classify --> Display/Buzzer
```
