# CARVIONICS - Avionic Display Layout

## Display Overview (128x160)

```
┌────────────────────────────┐
│  CARVIONICS        SYN/--- │  <- Header (16px)
├────────────────────────────┤
│  RPM      │      MAP       │
│  ----     │      ----      │  <- Primary Section (60px)
│ 5000 rpm  │  150 kpa       │
│           │                │
├────────────────────────────┤
│  CLT  │   AFR              │
│ 85 C  │   12.5             │  <- Secondary Section (42px)
│  IAT  │   BAT              │
│ 28 C  │   13.8 v           │
├────────────────────────────┤
│ TPS 45%        SyncLoss: 0 │  <- Footer (10px)
├────────────────────────────┤
│ NORMAL                     │  <- Status Bar (10px)
└────────────────────────────┘
```

## Layout Sections

### Header (0-16px)
- Project name: "CARVIONICS"
- Sync status indicator: "SYN" (green if synced) or "---" (red if lost)

### Primary Section (16-76px)
**Left Column (RPM)**
- Label: "RPM"
- Value: Engine RPM (large font for quick reading)

**Right Column (MAP/Boost)**
- Label: "MAP"
- Value: Manifold Absolute Pressure in kPa
- Color indicates boost level

### Secondary Section (76-118px)
**Left Column (Temperatures)**
- CLT: Coolant Temperature (Celsius)
- IAT: Intake Air Temperature (Celsius)

**Right Column (Efficiency & Power)**
- AFR: Air-Fuel Ratio (Lambda equivalent)
- BAT: Battery Voltage (Volts)

### Footer (118-128px)
- TPS: Throttle Position (Percentage)
- SyncLoss: Number of sync loss events
- Status indicator: "NORMAL", "CAUTION", "!WARNING", "RECOVERY", or "NO DATA"

## Color Coding

| Status | Color | Triggers |
|--------|-------|----------|
| Normal | Green | Engine healthy |
| Caution | Yellow | CLT > 90°C or AFR > 15.5 |
| Warning | Red | CLT > 95°C or AFR > 16.5 or BAT < 11.8V |
| Recovery | Yellow | Sync recovered, waiting for stability |
| No Data | Yellow | No serial communication |
| Sync Loss | Blinking Red | ECU sync lost |

### Parameter-Specific Colors

**CLT (Coolant Temperature)**
- Red: > 95°C
- Yellow: > 90°C
- Green: Normal

**IAT (Intake Temperature)**
- Yellow: > 60°C or < -5°C
- Green: Normal

**AFR (Air-Fuel Ratio)**
- Red: > 16.5 (lean)
- Yellow: > 15.5 or < 11.5 (slightly lean/rich)
- Green: Optimal

**MAP (Boost Pressure)**
- Red: > 200 kPa (over boost)
- Yellow: > 150 kPa (high boost)
- Green: Normal

**Battery Voltage**
- Red: < 11.8V (critical)
- Yellow: < 12.5V (low)
- Green: Normal

## Speeduino Data Mapping

The display reads from Speeduino serial protocol (115200 baud):

| Data | Source | Range |
|------|--------|-------|
| RPM | rxBuf[15:14] | 0-8000+ |
| MAP | rxBuf[17:16] | 0-255 kPa |
| CLT | rxBuf[18] | -40 to +125°C |
| IAT | rxBuf[19] | -40 to +125°C |
| TPS | rxBuf[24] | 0-100% |
| AFR | rxBuf[41:40] / 10 | 10.0-20.0 lambda |
| BAT | rxBuf[43:42] / 10 | 8.0-16.0V |
| SyncLoss | rxBuf[89:88] | 0-65535 |

## Sync Loss Detection

- Monitors ECU sync loss counter
- Triggers visual alarm when counter increases
- Recovery delay: 2.5 seconds after last sync loss
- Blinking red display during critical sync loss

## Update Strategy

- Only redraws sections with changed data (dirty flag system)
- Reduces TFT flicker and improves responsiveness
- No data timeout: 1000ms triggers "NO DATA" state

---

**Target MCU**: Arduino Mega 2560
**Display**: ST7735 1.8" 128x160 SPI TFT
**Baud Rate**: 115200
