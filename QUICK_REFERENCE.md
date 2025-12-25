# CARVIONICS Quick Reference

## 🚀 Fast Build Commands

### Build for UNO
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e uno
```

### Build for MEGA 2560
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e megaatmega2560
```

### Upload to UNO
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e uno --target upload
```

### Upload to MEGA 2560
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e megaatmega2560 --target upload
```

### Monitor Serial (UNO)
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe device monitor -e uno
```

### Monitor Serial (MEGA)
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe device monitor -e megaatmega2560
```

### Clean Build
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run --target clean
```

---

## 📌 Pin Configuration

### ST7735 Display → Arduino
```
Display  UNO   MEGA   Function
────────────────────────────────
VCC      5V    5V     Power
GND      GND   GND    Ground
CS       10    10     Chip Select
RST      9     9      Reset
DC       8     8      Data/Command
SDA      11    11     SPI MOSI
SCK      13    13     SPI Clock
LED      5V    5V     Backlight
```

### Speeduino ECU → Arduino
```
ECU      UNO        MEGA        Function
──────────────────────────────────────────
RX       Pin 0      Pin 18      Serial In
TX       Pin 1      Pin 19      Serial Out
GND      GND        GND         Ground
```

---

## 🔧 Configuration Files

### platformio.ini
```ini
[platformio]
default_envs = uno

[env:uno]
board = uno
monitor_port = COM3
upload_port = COM3

[env:megaatmega2560]
board = megaatmega2560
monitor_port = COM3
upload_port = COM3
```

### src/main.cpp (Key Defines)
```cpp
#define TFT_CS   10    // Chip Select
#define TFT_RST  9     // Reset
#define TFT_DC   8     // Data/Command

#if defined(ARDUINO_AVR_MEGA2560)
  #define ECU_SERIAL Serial1
#else
  #define ECU_SERIAL Serial
#endif
```

---

## 📊 Performance Stats

### Compilation
| Platform | Time | Result |
|----------|------|--------|
| UNO | 5.86s | ✅ SUCCESS |
| MEGA | 5.49s | ✅ SUCCESS |

### Memory Usage
| Platform | Flash | RAM |
|----------|-------|-----|
| UNO | 48.0% | 30.3% |
| MEGA | 6.4% | 7.6% |

### Features
- Real-time monitoring: 10 parameters
- Update frequency: ~10 Hz
- State detection: 6 modes
- Color schemes: 5 colors
- Display sections: 5 zones

---

## 🎨 Color Codes

```cpp
CLR_BG        = Black      (Background)
CLR_NORM      = Green      (Normal status)
CLR_INFO      = Cyan       (Information labels)
CLR_WARN      = Yellow     (Warning state)
CLR_CRIT      = Red        (Critical state)
CLR_TEXT      = White      (Text)
CLR_DARK_GREEN = Dark Green (Accents)
CLR_ORANGE    = Orange     (Accents)
CLR_DARK_CYAN = Dark Cyan  (Separators)
```

---

## ⚠️ Common Issues & Fixes

| Issue | Solution |
|-------|----------|
| Build fails | `pio run --target clean` |
| Upload fails | Verify USB cable & board |
| No display | Check pins 8, 9, 10 connections |
| No data | Check Speeduino at 115200 baud |
| COM not found | Check Device Manager |

---

## 📱 Display Sections

```
Section 1: Header (0-15px)
├─ Project name: "CARVIONICS"
└─ Sync status: "SYN" or "---"

Section 2: Primary (16-75px)
├─ Left: RPM display (dominant)
└─ Right: MAP pressure indicator

Section 3: Secondary (76-117px)
├─ Left: CLT, IAT (temperatures)
└─ Right: AFR, BAT (efficiency & power)

Section 4: Footer (118-128px)
├─ Left: TPS value
└─ Right: SyncLoss counter + Status
```

---

## 🔌 Power Supply

- **Recommended**: 5V / 1A USB power supply
- **Minimum**: 500mA @ 5V
- **Display draws**: ~100-150mA
- **Arduino draws**: ~50-100mA
- **Total**: ~200-250mA

---

## 📡 Serial Protocol

- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None
- **Flow Control**: None
- **Format**: Speeduino binary protocol
- **Frame Start**: 0xAA byte
- **Frame Size**: 128 bytes

---

## 🎯 Debug Tips

### Monitor Live Data
```bash
# Watch serial output in real-time
pio device monitor -e uno --baud 115200
```

### Check Compilation Details
```bash
# Verbose output
pio run -e uno -v
```

### Verify Memory Usage
```bash
# Shows detailed size info
pio run -e uno --verbose
```

### Clean Full Project
```bash
# Remove all build artifacts
pio run --target clean
```

---

## 🔄 Platform Switching Quick Guide

### VS Code (Easiest)
1. Click "uno" in bottom bar
2. Select target platform
3. Click build icon
4. Done! ✅

### Command Line (Fastest)
```bash
# One-liner for UNO
pio run -e uno --target upload ; pio device monitor -e uno

# One-liner for MEGA
pio run -e megaatmega2560 --target upload ; pio device monitor -e megaatmega2560
```

### Edit platformio.ini (Permanent)
```ini
[platformio]
default_envs = megaatmega2560  # Change default
```

---

## 📋 Pre-Flight Checklist

### Before Uploading
- [ ] Arduino connected via USB
- [ ] Correct board selected
- [ ] Correct COM port configured
- [ ] Code compiles without errors
- [ ] Display wiring verified
- [ ] Speeduino ECU ready

### After Upload
- [ ] Display shows startup
- [ ] Backlight is bright
- [ ] Serial data received ("SYN" appears)
- [ ] Engine parameters displaying
- [ ] Colors responding to values

---

## 🆘 Emergency Commands

### If USB port locks
```bash
# Force upload with reset
pio run -e uno --target upload --verbose
```

### If compiler cache corrupted
```bash
# Deep clean
pio run --target clean
pio run -e uno -v
```

### If board definition missing
```bash
# Reinstall platform
pio platform install atmelavr
```

---

## 📞 Quick Help

**Q: Which platform should I use?**  
A: Use UNO for development, MEGA for production.

**Q: Can I switch platforms later?**  
A: Yes! Code is 100% compatible. Just select different board.

**Q: How do I change the COM port?**  
A: Edit `platformio.ini`: `upload_port = COM3`

**Q: My display is blank?**  
A: Check pins 8, 9, 10 connections and power supply.

**Q: Speeduino not connecting?**  
A: Verify baud rate is 115200 in Speeduino config.

**Q: Can I modify the thresholds?**  
A: Yes! Edit `cltColor()`, `afrColor()` etc. in main.cpp

---

## 📚 Documentation Map

```
├── README.md                  ← Start here
├── WIRING_GUIDE.md           ← Hardware setup
├── DISPLAY_LAYOUT.md         ← Display info
├── PLATFORMIO_GUIDE.md       ← Build system
├── MULTI_PLATFORM_BUILD.md   ← Compilation info
├── PROJECT_SUMMARY.md        ← Complete overview
└── QUICK_REFERENCE.md        ← This file
```

---

## ⏱️ Quick Times

| Task | Time |
|------|------|
| Build UNO | 5.86s |
| Build MEGA | 5.49s |
| Upload UNO | 2-3s |
| Upload MEGA | 2-3s |
| Hardware setup | 5 min |
| Full project setup | 8 min |

---

**Last Updated**: December 25, 2025  
**Status**: ✅ Verified & Working
