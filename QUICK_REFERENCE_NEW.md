# Quick Reference - Carvionics EFIS

## Build Commands

```bash
# Mega 2560 (recommended)
pio run -e megaatmega2560

# Arduino Uno
pio run -e uno

# Upload
pio run -e megaatmega2560 -t upload
```

## Class Overview

| Class | Purpose |
|-------|---------|
| `ECUData` | Struct untuk engine parameters + validation |
| `SpeeduinoParser` | Serial frame parser (binary, 128 byte, 0xAA header) |
| `SyncManager` | State machine (NORMAL/CAUTION/WARNING/SYNC_LOSS/RECOVERY) |
| `DisplayManager` | TFT abstraction layer (MCUFRIEND wrapper) |
| `UIStateMachine` | Dirty flags + rendering orchestration |
| `UIScreen` | Render logic untuk setiap UI slice |

## Data Offsets (Speeduino v20+)

```
0-1:   RPM (uint16_t)
2:     CLT (int8_t, °C)
4-5:   AFR (uint16_t, 100x)
6:     MAP (uint8_t, kPa)
10:    TPS (uint8_t, %)
14:    IAT (int8_t, °C)
20:    Battery (uint8_t, 0.1V per unit)
31:    Status (bit 0 = sync)
```

## Configuration (Thresholds)

```cpp
SyncManager::Thresholds t;
t.rpm_max = 8000;
t.clt_min = 10; t.clt_max = 110;
t.afr_min = 1200; t.afr_max = 1700;
t.battery_min = 11000;  // 11V
t.data_timeout_ms = 500;
t.recovery_delay_ms = 2000;
sync_manager.setThresholds(t);
```

## Color Palette

```cpp
DisplayManager::Color::BLACK       // 0x0000
DisplayManager::Color::WHITE       // 0xFFFF
DisplayManager::Color::GREEN       // 0x07E0 (NORMAL)
DisplayManager::Color::AMBER       // 0xFDA0 (CAUTION)
DisplayManager::Color::RED         // 0xF800 (WARNING/SYNC_LOSS)
DisplayManager::Color::CYAN        // 0x07FF (AUX DATA)
DisplayManager::Color::DARK_GRAY   // 0x4208 (BACKGROUND)
```

## UI Layout (320×240)

```
┌──────────────────────────────────┐
│ HEADER (30px, DARK_GRAY)         │  y: 0-30
├──────────────────────────────────┤
│ RPM FIELD (90px, GREEN)          │  y: 30-120
│     5250 RPM                     │
├─────────────────┬────────────────┤
│ CLT/AFR (GREEN) │ MAP/BATT (GRN) │  y: 120-180
├─────────────────┼────────────────┤
│ TPS/IAT (CYAN)  │ FRAMES (CYAN)  │  y: 180-220
├──────────────────────────────────┤
│ FOOTER (20px, DARK_GRAY)         │  y: 220-240
└──────────────────────────────────┘
```

## State Transitions

```
[NORMAL] ←→ [CAUTION] ←→ [WARNING]
   ↓                        ↓
[SYNC_LOSS] ← data timeout / sync counter
   ↓
[RECOVERY] ← 2s delay
   ↓
[NORMAL]
```

## Key APIs

```cpp
// ECUData
ecu_data.isStale(500);  // timeout check
ecu_data.debugPrint();

// SpeeduinoParser
parser.begin(Serial1);
parser.update(ecu_data);
parser.getFramesReceived();

// SyncManager
sync_manager.update(ecu_data);
sync_manager.getState();
sync_manager.getStateString();
sync_manager.getRecoveryProgress();

// DisplayManager
display_.begin();
display_.fillScreen(Color::BLACK);
display_.print("Text");
display_.drawBox(x, y, w, h, border, fill);

// UIStateMachine
ui_state_machine.update(sync_state);
ui_state_machine.isSliceDirty(HEADER);
ui_state_machine.markSliceClean(HEADER);

// UIScreen
ui_screen.render(ecu_data, sync_mgr, ui_state);
```

## Memory Usage

```
Mega 2560:
  Used:  1372 bytes / 8192 (16.7%)
  Flash: 16280 bytes / 253952 (6.4%)

Arduino Uno:
  Used:  1215 bytes / 2048 (59.3%)
  Flash: 15598 bytes / 32256 (48.4%)
```

## Common Tasks

### Enable Debug Output
```cpp
// Di loop, every 5 seconds:
ecu_data.debugPrint();
parser.debugPrint();
sync_manager.debugPrint();
```

### Change Threshold Dynamically
```cpp
SyncManager::Thresholds t = sync_manager.getThresholds();
t.rpm_max = 8500;
sync_manager.setThresholds(t);
```

### Force Sync Loss
```cpp
sync_manager.triggerSyncLoss();
```

### Clear Display
```cpp
display_.clear();  // fillScreen(BLACK)
```

## Default Values

```cpp
rpm: 0
clt: 0
afr: 1400 (14.00:1)
map: 0
tps: 0
iat: 0
battery: 12000 (12V)
syncLossCounter: 0
```

## Serial Communication

```
Baud: 115200
Parity: None (8N1)
Frame: 128 byte
Header: 0xAA (byte 0)
Mode: Passive (read-only, no response)
Direction: Speeduino ECU → Arduino RX
```

## File Structure

```
include/
  ├── SimpleLCD.h        (umbrella header)
  ├── ECUData.h
  ├── SpeeduinoParser.h
  ├── SyncManager.h
  ├── DisplayManager.h
  ├── UIStateMachine.h
  └── UIScreen.h
src/
  ├── main.cpp
  └── lib/
      ├── ECUData.cpp
      ├── SpeeduinoParser.cpp
      ├── SyncManager.cpp
      ├── DisplayManager.cpp
      ├── UIStateMachine.cpp
      └── UIScreen.cpp
```

## Notes

- ✅ Both platforms compile successfully
- ✅ OOP architecture fully modular
- ✅ Non-blocking serial parser (no delays)
- ✅ Dirty-flag rendering (anti-flicker)
- ✅ Safe state machine with recovery
- ✅ Ready for production use

---
**Version**: 3.0.0 | **Date**: 25 Dec 2025 | **Status**: ✅ Complete
