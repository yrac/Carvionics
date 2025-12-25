# CARVIONICS - Wiring Guide

## ST7735 1.8" Display Pinout

### Display Pin Configuration

```
ST7735 Display (8-pin header)
┌─────────────────┐
│ VCC GND CS RST  │ (Top row)
│ DC  SDA SCK LED │ (Bottom row)
└─────────────────┘
```

## Arduino UNO Wiring (Recommended)

### Pinout Table

| ST7735 Pin | Function | Arduino UNO Pin | Wire Color |
|-----------|----------|-----------------|-----------|
| VCC | Power 5V | 5V | Red |
| GND | Ground | GND | Black |
| CS | Chip Select | 10 | Orange |
| RST | Reset | 9 | Yellow |
| DC | Data/Command | 8 | Green |
| SDA | SPI MOSI | 11 | Blue |
| SCK | SPI Clock | 13 | Purple |
| LED | Backlight | 5V or Pin 6 | White |

### Connection Diagram

```
ST7735 Display
│
├─ VCC ──────────────→ Arduino 5V
├─ GND ──────────────→ Arduino GND
├─ CS  ──────────────→ Arduino Pin 10
├─ RST ──────────────→ Arduino Pin 9
├─ DC  ──────────────→ Arduino Pin 8
├─ SDA ──────────────→ Arduino Pin 11 (SPI MOSI)
├─ SCK ──────────────→ Arduino Pin 13 (SPI Clock)
└─ LED ──────────────→ Arduino 5V (or PWM Pin 6 for brightness control)

Optional: Add 100nF (0.1µF) decoupling capacitor between VCC and GND
```

## Arduino MEGA 2560 Wiring (Alternative)

If you prefer to use MEGA-specific pins, change the `#define` values in `main.cpp`:

| ST7735 Pin | Function | Arduino MEGA Pin | Current Setting |
|-----------|----------|-----------------|-----------------|
| VCC | Power 5V | 5V | - |
| GND | Ground | GND | - |
| CS | Chip Select | 53 | 10 |
| RST | Reset | 48 | 9 |
| DC | Data/Command | 49 | 8 |
| SDA | SPI MOSI | 51 | 11 |
| SCK | SPI Clock | 52 | 13 |
| LED | Backlight | 5V | - |

### To use MEGA pins, modify in main.cpp:

```cpp
// MEGA Configuration (if needed)
#define TFT_CS   53    // Change from 10
#define TFT_RST  48    // Change from 9
#define TFT_DC   49    // Change from 8
```

## SPI Bus Information

### Arduino UNO SPI Pins
- **MOSI (Master Out Slave In)**: Pin 11 (ICSP pin 4)
- **MISO (Master In Slave Out)**: Pin 12 (ICSP pin 1)
- **SCK (Serial Clock)**: Pin 13 (ICSP pin 3)
- **Hardware SPI is used automatically by library**

### Arduino MEGA 2560 SPI Pins
- **MOSI**: Pin 51 (ICSP pin 4)
- **MISO**: Pin 12 (ICSP pin 1)
- **SCK**: Pin 52 (ICSP pin 3)
- **Hardware SPI is used automatically by library**

## Current Configuration

```cpp
/* main.cpp Pin Configuration */
#define TFT_CS   10    // Chip Select (UNO compatible)
#define TFT_RST  9     // Reset
#define TFT_DC   8     // Data/Command
```

**✅ Fully compatible with Arduino UNO**
**✅ Works with Arduino MEGA 2560** (uses pins 10, 9, 8 which are available on both)

## Speeduino Connection

The code reads from Speeduino ECU via Serial:

| Component | Arduino Pin | Baud Rate |
|-----------|------------|-----------|
| Speeduino RX | Serial1 RX | 115200 |
| Speeduino TX | Serial1 TX | 115200 |

**For Arduino UNO**: Use Software Serial on pins 0 & 1 (hardware UART)
**For Arduino MEGA**: Use Serial1 (pins 18 & 19) - recommended

### Current Configuration in code:
```cpp
Serial1.begin(115200);  // MEGA: Hardware Serial1
```

### For Arduino UNO (if using MEGA code):
```cpp
Serial.begin(115200);   // UNO: Hardware Serial (pins 0, 1)
```

## Power Requirements

- **Display Power**: 100-150mA @ 5V
- **Arduino Power**: 50-100mA
- **Total**: ~200-250mA @ 5V

**Recommended**: Use 5V/1A+ USB power supply or dedicated power supply

## Optional: Brightness Control

To add PWM brightness control on pin 6:

```cpp
// In setup():
pinMode(6, OUTPUT);

// In loop() (optional):
analogWrite(6, 200);  // 0-255 brightness level
```

Connect ST7735 LED pin to Arduino pin 6 via 100Ω resistor if desired.

## Testing Connections

1. **Power on**: Display should show startup splash
2. **Backlight**: LED should light up
3. **Display content**: Should show CARVIONICS header once synced
4. **Serial**: Check sync indicator in top-right corner

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| Display blank/white | Wrong pin assignment | Check #define values |
| Display not responding | SPI conflict | Check other SPI devices |
| Partial display | Poor connection | Resolder wires |
| No backlight | LED pin not connected | Connect LED to 5V or PWM pin |
| Serial data not reading | Wrong baud rate | Check Speeduino output: 115200 |

---

**Last Updated**: December 2025
**Compatible MCU**: Arduino UNO, MEGA 2560, Pro Mini
**Display**: ST7735 1.8" 128x160 SPI TFT
