# CARVIONICS - PlatformIO Configuration Guide

## Project Overview

CARVIONICS adalah automotive display yang kompatibel dengan dua platform Arduino:
- **Arduino UNO** (Default)
- **Arduino MEGA 2560** (Alternative)

## Platform Configuration

### Current Setup

```ini
[platformio]
default_envs = uno
```

Default environment adalah **UNO**. Anda dapat menggantinya dengan `megaatmega2560` jika menggunakan MEGA.

## Switching Between Platforms

### Method 1: Using VS Code Bottom Bar

1. Klik pada "uno" atau "megaatmega2560" di bottom bar PlatformIO
2. Select environment yang diinginkan
3. Build & Upload otomatis menggunakan environment yang dipilih

### Method 2: Using PlatformIO CLI

```bash
# Build untuk Arduino UNO
pio run -e uno

# Build untuk Arduino MEGA 2560
pio run -e megaatmega2560

# Upload ke Arduino UNO
pio run -e uno --target upload

# Upload ke Arduino MEGA 2560
pio run -e megaatmega2560 --target upload

# Monitor serial untuk UNO
pio device monitor -e uno

# Monitor serial untuk MEGA
pio device monitor -e megaatmega2560
```

## Comparing Platforms

| Feature | Arduino UNO | Arduino MEGA 2560 |
|---------|------------|------------------|
| **Processor** | ATmega328P | ATmega2560 |
| **Clock Speed** | 16 MHz | 16 MHz |
| **Flash Memory** | 32 KB | 256 KB |
| **RAM** | 2 KB | 8 KB |
| **Serial Ports** | 1 (pins 0,1) | 4 (Serial, Serial1-3) |
| **SPI Pins** | 11, 12, 13 | 50, 51, 52 |
| **Cost** | $$ | $$$$ |
| **Size** | Compact | Large |

### CARVIONICS Pin Usage

Both platforms use the **same logical pins** in code:

```cpp
#define TFT_CS   10    // SPI Chip Select
#define TFT_RST  9     // Display Reset
#define TFT_DC   8     // Data/Command
// SPI MOSI: 11
// SPI SCK:  13
// Serial RX: 0
// Serial TX: 1 (UNO) or 18 (MEGA Serial1)
```

**Note**: Both platforms have these pins available, making the code truly portable!

## Configuration Details

### Arduino UNO Configuration
```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps = 
	adafruit/Adafruit GFX Library@^1.12.4
	adafruit/Adafruit ST7735 and ST7789 Library@^1.11.0
build_flags = 
	-DSERIAL_BAUD=115200
	-DARDUINO_BOARD=CARVIONICS_v1
monitor_speed = 115200
upload_speed = 115200
monitor_port = COM3        # Adjust for your system
upload_port = COM3         # Adjust for your system
```

### Arduino MEGA 2560 Configuration
```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
framework = arduino
lib_deps = 
	adafruit/Adafruit GFX Library@^1.12.4
	adafruit/Adafruit ST7735 and ST7789 Library@^1.11.0
build_flags = 
	-DSERIAL_BAUD=115200
	-DARDUINO_BOARD=CARVIONICS_v1
monitor_speed = 115200
upload_speed = 115200
monitor_port = COM3        # Adjust for your system
upload_port = COM3         # Adjust for your system
```

## Serial Communication Configuration

### For Arduino UNO

Default UART uses Serial (pins 0, 1):

```cpp
Serial.begin(115200);  // Read from Speeduino
```

### For Arduino MEGA 2560

Can use Serial1 (pins 18, 19) for Speeduino:

```cpp
Serial1.begin(115200);  // Read from Speeduino
```

**Current code uses**: `Serial1.begin(115200)` 
- This is compatible with MEGA
- For UNO, would need to change to `Serial.begin(115200)`

## Adjusting COM Ports

The `monitor_port` and `upload_port` in `platformio.ini` may need adjustment:

### Windows
```ini
monitor_port = COM3
upload_port = COM3
```

### Linux
```ini
monitor_port = /dev/ttyUSB0
upload_port = /dev/ttyUSB0
```

### macOS
```ini
monitor_port = /dev/cu.usbserial-*
upload_port = /dev/cu.usbserial-*
```

**To find your port**:
- Windows: Device Manager → Ports (COM & LPT)
- Linux: `ls /dev/tty*`
- macOS: `ls /dev/cu.*`

## Memory Usage

### Arduino UNO (32 KB Flash, 2 KB RAM)
- ⚠️ **Limited**: Code may need optimization for UNO
- Adafruit libraries use significant space
- May struggle with complex graphics

### Arduino MEGA 2560 (256 KB Flash, 8 KB RAM)
- ✅ **Plenty of space**: No memory concerns
- All features fully supported
- Room for future expansion

## Recommended Configuration

### For Development/Testing:
→ **Arduino UNO** (cheaper, faster compile)

### For Production/Deployment:
→ **Arduino MEGA 2560** (more reliable, more features)

## Building the Project

### Build Current Environment
```bash
cd D:\Solution\Carvionics
pio run
```

### Build All Environments
```bash
pio run -e uno
pio run -e megaatmega2560
```

### Clean and Rebuild
```bash
pio run --target clean
pio run --target upload
```

## Troubleshooting

### "Board not found"
- Check board selection in bottom bar
- Verify USB connection
- Check Device Manager for COM port

### "Serial port not open"
- Update COM port in `platformio.ini`
- Ensure Arduino drivers are installed
- Check USB cable quality

### "Upload failed"
- Verify correct board selected
- Check upload speed (default: 115200)
- Try different USB port

### "Compilation error"
- Ensure all libraries are installed
- Check PlatformIO environment is synced
- Review error messages for specific issues

## Future Enhancements

Possible upgrades untuk future versions:
1. **Arduino NANO**: Smaller, same capabilities as UNO
2. **Arduino DUE**: 32-bit, faster processor
3. **Custom STM32**: More powerful alternative
4. **Multi-board support**: Auto-detect board

---

**Last Updated**: December 2025
**Supported Boards**: Arduino UNO, MEGA 2560
**Default Board**: UNO
**Framework**: Arduino
**Build System**: PlatformIO
