# 🚗 Carvionics EFIS - Display Engine Speeduino

**Sistem display mesin bergaya cockpit pesawat untuk ECU Speeduino**

> Proyek Arduino Open Source yang memantau real-time data mesin Anda di layar TFT 320×240 dengan gaya avionics yang keren. Dirancang khusus untuk Speeduino ECU menggunakan Arduino Mega atau Uno.

---

## 📌 Daftar Isi

- [Apa itu Carvionics?](#apa-itu-carvionics)
- [Fitur Unggulan](#fitur-unggulan)
- [Mockup Tampilan](#mockup-tampilan)
- [Hardware yang Dibutuhkan](#hardware-yang-dibutuhkan)
- [Setup & Instalasi](#setup--instalasi)
- [Cara Menggunakan](#cara-menggunakan)
- [Struktur Kode](#struktur-kode)
- [Troubleshooting](#troubleshooting)

---

## ❓ Apa itu Carvionics?

Carvionics EFIS adalah sistem display untuk memonitor kondisi mesin kendaraan Anda secara **real-time**. 

### 🎯 Fitur Utama

- 📡 **Passive Listener** - Hanya membaca data dari ECU, tidak mengirim apapun (aman & terpercaya)
- 🎨 **Avionics Style** - Tampilannya seperti cockpit pesawat, profesional & mudah dibaca
- ⚡ **Fast Update** - Data diperbarui 20 kali per detik (20Hz) untuk response real-time
- 🛡️ **Safety Critical** - Deteksi otomatis sync loss & blink alert untuk koneksi putus
- 🎯 **OOP Architecture** - Kode modular, mudah di-extend & maintain
- 🎛️ **Dual Platform** - Kompatibel Arduino Mega 2560 (recommended) atau Uno (hemat)

**Dalam bahasa sederhana:** Carvionics adalah "avionics instrument panel" untuk mesin Speeduino Anda! 🚗💨

---

## 🌟 Fitur Unggulan

| Fitur | Deskripsi |
|-------|-----------|
| **Six-Pack Layout** | Grid 3×2 seperti flight instruments (RPM, MAP, CLT, IAT, AFR, TPS) |
| **Color-Coded Values** | GREEN (normal) / AMBER (caution) / RED (warning) berubah dinamis |
| **State Machine** | NO_DATA → NORMAL → CAUTION → WARNING → SYNC_LOSS → RECOVERY |
| **Visual Alert** | Full-screen red blink saat sync loss terdeteksi |
| **Smooth Fonts** | FreeFonts untuk Mega 2560, built-in font untuk UNO |
| **Data Validation** | Timeout detection, threshold checks, auto-recovery |
| **Dirty Flag Optimization** | Per-slice redraw untuk anti-flicker & smooth animation |
| **Dual Platform** | UNO (80% RAM, 89% Flash) atau Mega (22% RAM, 17% Flash) |

---

## 📱 Mockup Tampilan

### Status: NORMAL (Green - All Systems GO)
```
╔══════════════════════════════════════════════╗
║ ECU: OK    SYNC: 0    BAT: 13.8V            ║
╠══════════════════════════════════════════════╣
║                                              ║
║  ┌──────────┬──────────┬──────────┐          ║
║  │   RPM    │   MAP    │   CLT    │          ║
║  │          │          │          │          ║
║  │  5850    │   65     │   85     │ GREEN   ║
║  │          │          │          │          ║
║  └──────────┴──────────┴──────────┘          ║
║  ┌──────────┬──────────┬──────────┐          ║
║  │   IAT    │   AFR    │   TPS    │          ║
║  │          │          │          │          ║
║  │   32     │  14.2    │   25     │ GREEN   ║
║  │          │          │          │          ║
║  └──────────┴──────────┴──────────┘          ║
║                                              ║
╠══════════════════════════════════════════════╣
║ NORMAL                       FUEL: --.- L   ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Six-Pack Avionics Layout
```

### Status: WARNING (Red - Critical Alert!)
```
╔══════════════════════════════════════════════╗
║ ECU: OK    SYNC: 0    BAT: 11.2V  ⚠️        ║
╠══════════════════════════════════════════════╣
║                                              ║
║  ┌──────────┬──────────┬──────────┐          ║
║  │   RPM    │   MAP    │   CLT    │          ║
║  │          │          │          │          ║
║  │  7850    │   88     │  113     │ RED     ║
║  │  RED     │  AMBER   │  RED!    │          ║
║  └──────────┴──────────┴──────────┘          ║
║  ┌──────────┬──────────┬──────────┐          ║
║  │   IAT    │   AFR    │   TPS    │          ║
║  │          │          │          │          ║
║  │   38     │  11.8    │   65     │ RED     ║
║  │  AMBER   │  RED!    │  AMBER   │          ║
║  └──────────┴──────────┴──────────┘          ║
║                                              ║
╠══════════════════════════════════════════════╣
║ WARNING                      FUEL: --.- L   ║
╚══════════════════════════════════════════════╝
        Color-Coded Values Change Dynamically!
```

### Status: SYNC_LOSS (Full Screen Blink Alert)
```
╔══════════════════════════════════════════════╗
║                                              ║
║                                              ║
║                   SYNC                       ║
║                   LOSS                       ║
║                 NO ECU                       ║
║                                              ║
║             Recovery: 75%                    ║
║                                              ║
║                                              ║
╚══════════════════════════════════════════════╝
        Background: RED ⇄ BLACK (Blink)
        Font Size 4 untuk SYNC/LOSS
        Size 3 untuk NO ECU
        Size 2 untuk Recovery
```
║  Engine temperature approaching limit       ║
║                                              ║
║          STATUS: ⚠️ CAUTION                  ║
║                                              ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Background Color: AMBER/YELLOW
```

### Status: WARNING (Red - Critical!)
```
╔══════════════════════════════════════════════╗
║                                              ║
║              🔴 WARNING 🔴                   ║
║                                              ║
║              ┌─────────────────┐             ║
║              │      5850       │             ║
║              │      RPM        │             ║
║              └─────────────────┘             ║
║                                              ║
║         ┌──────────┬──────────┐              ║
║         │   CLT    │   AFR    │              ║
║         │🔴105°C🔴 │  13.2:1  │              ║
║         ├──────────┼──────────┤              ║
║         │   MAP    │   BAT    │              ║
║         │ 650 kPa  │🔴11.2V🔴 │              ║
║         └──────────┴──────────┘              ║
║                                              ║
║  CRITICAL: Temperature exceeded!             ║
║  CRITICAL: Battery voltage too low!          ║
║                                              ║
║          STATUS: 🔴 WARNING                  ║
║                                              ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Background Color: RED (Dark)
        Some fields blinking RED
```

### Status: SYNC LOSS (Red Blink - Connection Lost!)
```
╔══════════════════════════════════════════════╗
║                                              ║
║          ❌ SYNC LOSS - NO DATA ❌           ║
║                                              ║
║                                              ║
║          Connection timeout detected!        ║
║                                              ║
║      Last data received: 2.5 seconds ago     ║
║                                              ║
║          Check Speeduino ECU cable!          ║
║                                              ║
║          Waiting for reconnection...         ║
║                                              ║
║      (Screen is BLINKING RED continuously)  ║
║                                              ║
║                                              ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Background Color: RED (Blinking)
        Entire screen flashing every 200ms
```

### Status: NO DATA (Black - Waiting for First Connection)
```
╔══════════════════════════════════════════════╗
║                                              ║
║                                              ║
║                    ⏳                         ║
║                                              ║
║         Waiting for ECU connection...        ║
║                                              ║
║      Check serial cable and baud rate       ║
║                                              ║
║                                              ║
║                                              ║
║                                              ║
║                                              ║
║                                              ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Background Color: BLACK
        Status: Initializing...
```

---

## 🖥️ Avionics Mockup per State

Berikut rancangan layar per state sesuai aturan avionics (truth-first, tidak berlebihan, fokus ke angka penting):

### 1️⃣ BOOT / POWER ON

Tujuan: alat hidup & hardware OK

```
┌──────────────────────────────────────────────┐
│ ECU MONITOR                                  │
│                                              │
│ TFT   OK                                     │
│ MCU   OK                                     │
│ BUS   OK                                     │
│                                              │
│ INIT...                                      │
└──────────────────────────────────────────────┘
```

- Tidak ada data mesin
- Tidak ada animasi
- Mirip avionics self-test

### 2️⃣ WAIT ECU (NO DATA)

Tujuan: jujur — ECU belum kirim data

```
┌──────────────────────────────────────────────┐
│ ECU MONITOR                                  │
│                                              │
│ WAIT ECU                                     │
│ SERIAL LINK                                  │
│                                              │
│ RPM ---   CLT ---   AFR ---                  │
└──────────────────────────────────────────────┘
```

- Placeholder --- → tidak bohong
- User tahu ini normal

### 3️⃣ SYNCING (DATA MASUK, BELUM STABIL)
```
┌──────────────────────────────────────────────┐
│ ECU            SYNCING...                    │
├──────────────────────────────────────────────┤
│ RPM ----        MAP --                       │
│ CLT --          IAT --                       │
│ AFR --          TPS --                       │
└──────────────────────────────────────────────┘
```

- Data belum valid
- Tidak tampil ignition / idle
- Transisi cepat ke NORMAL

### 4️⃣ NORMAL OPERATION (MAIN DISPLAY - Six-Pack Layout)
```
┌──────────────────────────────────────────────┐
│ ECU: OK    SYNC: 0    BAT: 13.9V            │
├──────────────────────────────────────────────┤
│ ┌──────────┬──────────┬──────────┐          │
│ │   RPM    │   MAP    │   CLT    │          │
│ │  2450    │   42     │   87     │  GREEN  │
│ └──────────┴──────────┴──────────┘          │
│ ┌──────────┬──────────┬──────────┐          │
│ │   IAT    │   AFR    │   TPS    │          │
│ │   31     │  14.4    │   3      │  GREEN  │
│ └──────────┴──────────┴──────────┘          │
├──────────────────────────────────────────────┤
│ NORMAL                       FUEL: --.- L   │
└──────────────────────────────────────────────┘
```

- Grid 3×2 equal-sized cells (six-pack avionics)
- Label kecil di kiri atas setiap cell
- Value besar di tengah, color-coded
- No borders between cells (clean look)

### 5️⃣ CAUTION STATE (Yellow Alert)

Contoh: CLT tinggi / AFR lean

```
┌──────────────────────────────────────────────┐
│ ECU: OK    SYNC: 0    BAT: 13.4V            │
├──────────────────────────────────────────────┤
│ ┌──────────┬──────────┬──────────┐          │
│ │   RPM    │   MAP    │   CLT    │          │
│ │  2450    │   42     │   97     │  AMBER  │
│ │  GREEN   │  GREEN   │  AMBER!  │         │
│ └──────────┴──────────┴──────────┘          │
│ ┌──────────┬──────────┬──────────┐          │
│ │   IAT    │   AFR    │   TPS    │          │
│ │   31     │  16.2    │   3      │  AMBER  │
│ │  GREEN   │  AMBER!  │  GREEN   │         │
│ └──────────┴──────────┴──────────┘          │
├──────────────────────────────────────────────┤
│ CAUTION                      FUEL: --.- L   │
└──────────────────────────────────────────────┘
```

- Values berubah warna AMBER jika melampaui threshold
- Footer menunjukkan state CAUTION
- Angka tetap tampil, tidak blink

### 6️⃣ WARNING STATE (Red Alert!)

Contoh: Overheat / battery drop

```
┌──────────────────────────────────────────────┐
│ ECU: OK    SYNC: 0    BAT: 11.8V  ⚠️        │
├──────────────────────────────────────────────┤
│ ┌──────────┬──────────┬──────────┐          │
│ │   RPM    │   MAP    │   CLT    │          │
│ │  2600    │   55     │  108     │  RED    │
│ │  RED!    │  AMBER   │  RED!!   │         │
│ └──────────┴──────────┴──────────┘          │
│ ┌──────────┬──────────┬──────────┐          │
│ │   IAT    │   AFR    │   TPS    │          │
│ │   38     │  15.9    │   12     │  RED    │
│ │  AMBER   │  RED!    │  AMBER   │         │
│ └──────────┴──────────┴──────────┘          │
├──────────────────────────────────────────────┤
│ WARNING                      FUEL: --.- L   │
└──────────────────────────────────────────────┘
```

- Critical values tampil RED
- Multiple parameters warning
- State footer: WARNING

### 7️⃣ SYNC LOSS (FULL SCREEN ALERT)

Koneksi ECU putus → full-screen blink

```
┌──────────────────────────────────────────────┐
│                                              │
│                                              │
│                   SYNC                       │
│                   LOSS                       │
│                 NO ECU                       │
│                                              │
│             Recovery: 45%                    │
│                                              │
╚══════════════════════════════════════════════╝
        Background: RED ⇄ BLACK (300ms cycle)
        Text Size 4: SYNC/LOSS
        Text Size 3: NO ECU
        Text Size 2: Recovery progress
```

- Fullscreen override untuk safety-critical alert
- Blink 300ms on/off untuk menarik perhatian
- Recovery progress menunjukkan kapan kembali normal

### 8️⃣ RECOVERY (RECONNECTING)

Setelah sync loss → data kembali → tunggu stabilisasi

```
┌──────────────────────────────────────────────┐
│ ECU: OK  75%  SYNC: 0    BAT: 13.6V         │
├──────────────────────────────────────────────┤
│ ┌──────────┬──────────┬──────────┐          │
│ │   RPM    │   MAP    │   CLT    │          │
│ │   --     │   --     │   --     │  AMBER  │
│ └──────────┴──────────┴──────────┘          │
│ ┌──────────┬──────────┬──────────┐          │
│ │   IAT    │   AFR    │   TPS    │          │
│ │   --     │   --     │   --     │  AMBER  │
│ └──────────┴──────────┴──────────┘          │
├──────────────────────────────────────────────┤
│ RECOVERY                     FUEL: --.- L   │
└──────────────────────────────────────────────┘
```

- Placeholder "--" saat menunggu data valid
- Recovery progress di header
- Transisi smooth ke NORMAL setelah 100%

### 🧭 Ringkasan State Machine

```
BOOT (Self Test)
 ↓
WAIT_ECU (No Data)
 ↓
SYNCING (Data Arriving)
 ↓
NORMAL (Green)
 ├→ CAUTION (Amber)
 ├→ WARNING (Red)
 └→ SYNC_LOSS → RECOVERY → NORMAL
```

**Thresholds yang Dimonitor:**
- RPM: Max 8000
- CLT: Min 10°C, Max 110°C
- AFR: Min 12.0, Max 17.0
- Battery: Min 11.0V

---

## 🔧 Hardware yang Dibutuhkan

### 📦 Komponen Utama

Anda membutuhkan 3 komponen inti:

```
┌─────────────────────────────────────┐
│  Arduino Mega 2560 (RECOMMENDED)    │  ← Microcontroller
│  - 248KB Flash (17% usage)          │
│  - 8KB RAM (22% usage)              │
│  - Smooth FreeFonts support         │
│                                     │
│  OR Arduino Uno (Budget Option)     │
│  - 32KB Flash (89% usage)           │
│  - 2KB RAM (80% usage)              │
│  - Built-in fonts only              │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│  TFT 2.4" Display 320×240           │  ← Display
│  Parallel Interface (8-bit)         │
│  ILI9325 / ILI9341 Controller       │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│  Speeduino ECU                      │  ← Data Source
│  Serial Output (UART)               │
│  115200 baud, 8N1                   │
└─────────────────────────────────────┘
```

### 📋 Spesifikasi Hardware

| Komponen | Spesifikasi | Catatan |
|----------|-------------|---------|
| **MCU** | ATmega2560 16MHz | Mega: 8KB SRAM, Uno: 2KB SRAM |
| **TFT Display** | 2.4" 320×240 pixels | 8-bit parallel (16-pin data) |
| **Serial Link** | 115200 baud, 8N1 | Passive read-only |
| **Power Supply** | 7-12V external / USB | Display: 5V regulated |
| **RAM Usage** | ~1.4 KB (Mega), ~1.2 KB (Uno) | Very efficient |
| **Flash Usage** | ~16 KB code | Plenty of space remaining |

### 🔌 Wiring (Arduino Mega - Recommended)

```
┌────────────────────────────────────────────────┐
│   TFT Display Pins → Arduino Mega Pins         │
├────────────────────────────────────────────────┤
│   Data Bus (8 parallel lines):                 │
│     D0 → A0    |  D4 → A4                      │
│     D1 → A1    |  D5 → A5                      │
│     D2 → A2    |  D6 → A6                      │
│     D3 → A3    |  D7 → A7                      │
├────────────────────────────────────────────────┤
│   Control Lines:                               │
│     CS  (Chip Select)  → Digital Pin 3         │
│     CD  (Command/Data) → Digital Pin 2         │
│     WR  (Write)        → Digital Pin 4         │
│     RD  (Read)         → Digital Pin 5         │
│     RST (Reset)        → Digital Pin 6         │
├────────────────────────────────────────────────┤
│   Power:                                       │
│     VCC → 5V (via regulator)                   │
│     GND → GND (common ground ALL devices)      │
├────────────────────────────────────────────────┤
│   Serial from Speeduino ECU:                   │
│     ECU TX → Arduino RX1 (Pin 19 - Mega)       │
│     ECU GND → Arduino GND (common ground)      │
└────────────────────────────────────────────────┘
```

> 💡 **Tips Penting:** Gunakan kabel berkualitas & grounding yang bagus sangat penting untuk data integrity!

---

## 🚀 Setup & Instalasi

### 1️⃣ Install PlatformIO

**Option A: Langsung dari VSCode (Recommended)**
- Install extension "PlatformIO IDE" dari marketplace
- Restart VSCode
- PlatformIO auto-download tools yang diperlukan

**Option B: Manual via Terminal**
```bash
pip install platformio
```

### 2️⃣ Clone Repository

```bash
git clone https://github.com/YourUsername/Carvionics.git
cd Carvionics
```

### 3️⃣ Update Configuration (platformio.ini)

Edit `platformio.ini` sesuai board Anda:

**Untuk Arduino Mega:**
```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
upload_port = COM3        ; ← Update dengan port Anda
monitor_port = COM3
upload_speed = 115200
```

**Untuk Arduino Uno:**
```ini
[env:uno]
platform = atmelavr
board = uno
upload_port = COM5        ; ← Update dengan port Anda
monitor_port = COM5
upload_speed = 115200
```

### 4️⃣ Compile & Upload

**Via VSCode (Paling Mudah):**
1. Buka folder project di VSCode
2. PlatformIO akan auto-detect `platformio.ini`
3. Klik ikon PlatformIO di sidebar
4. Pilih environment (uno atau megaatmega2560)
5. Click "Upload" atau tekan `Ctrl+Alt+U`

**Via Terminal:**
```bash
# Build for Mega:
pio run -e megaatmega2560

# Build for UNO:
pio run -e uno

# Upload to Mega:
pio run -e megaatmega2560 --target upload

# Upload to UNO:
pio run -e uno --target upload

# Run demo test (UNO):
pio test -e uno
```

**Memory Usage Report:**
```
Arduino UNO:
- RAM:   80.5% (1648/2048 bytes)
- Flash: 89.6% (28892/32256 bytes)
- Font:  Built-in bitmap font
- Status: ✅ Stable

Arduino Mega 2560:
- RAM:   22.0% (1805/8192 bytes)
- Flash: 17.1% (43456/253952 bytes)
- Font:  FreeSansBold (smooth)
- Status: ✅ Recommended (more headroom)
```

# Uno:
pio run -e uno --target upload
```

✅ **Done!** Layar akan hidup dan menampilkan data mesin.

---

## 💻 Cara Menggunakan

### 🎯 Display States & Color Coding

Carvionics memiliki state machine dengan visual feedback yang jelas:

| State | Display | Color | Arti | Action |
|-------|---------|-------|------|--------|
| **BOOT** | Self-test screen | White | Inisialisasi sistem | Wait ~2 detik |
| **WAIT_ECU** | No Data placeholder | Amber | Menunggu ECU | Check kabel serial |
| **SYNCING** | Grid dengan "--" | Amber | Data arriving | Wait stabilisasi |
| **NORMAL** | Six-pack green | GREEN | Semua OK ✅ | Monitor biasa |
| **CAUTION** | Values amber | AMBER | Warning threshold ⚠️ | Perhatian |
| **WARNING** | Values red | RED | Critical alert 🔴 | Tindakan segera! |
| **SYNC_LOSS** | Full screen blink | RED/BLACK | Koneksi putus ❌ | Check trigger |
| **RECOVERY** | Placeholder "--" | AMBER | Reconnecting | Wait 2.5 detik |

**Color-Coded Values:**
- 🟢 **GREEN**: Nilai dalam range normal
- 🟡 **AMBER**: Melampaui threshold caution
- 🔴 **RED**: Melampaui threshold warning

**Layout:**
- Header: ECU status, SYNC counter, Battery voltage
- Grid 3×2: RPM, MAP, CLT (row 1) / IAT, AFR, TPS (row 2)
- Footer: State name, FUEL placeholder

### 🎮 Tombol & Interaksi

Saat ini adalah **passive display** - hanya menampilkan data tanpa button interaksi.

Future features:
- Button untuk toggle display modes
- Custom threshold settings via config
- Data logging to SD card

---

## 📂 Struktur Kode

```
Carvionics/
├── src/
│   ├── main.cpp                     ← Entry point (setup & main loop)
│   └── lib/
│       ├── DisplayManager.cpp       ← TFT abstraction & color palette
│       ├── ECUData.cpp              ← Data structures & validation
│       ├── SpeeduinoParser.cpp      ← Serial protocol parser
│       ├── SyncManager.cpp          ← Threshold monitoring & state logic
│       ├── UIScreen.cpp             ← All rendering logic (six-pack layout)
│       └── UIStateMachine.cpp       ← State transitions & dirty flags
├── include/
│   ├── DisplayManager.h
│   ├── ECUData.h
│   ├── SimpleLCD.h                  ← (Legacy, unused)
│   ├── SpeeduinoParser.h
│   ├── SyncManager.h
│   ├── UIScreen.h
│   └── UIStateMachine.h
├── test/
│   └── test_ui_demo.cpp             ← Animated demo test (40 detik)
├── platformio.ini                   ← Build configuration
├── README.md                        ← English documentation
├── README_ID.md                     ← Dokumentasi Bahasa Indonesia (ini)
├── WIRING_GUIDE.md                  ← Diagram koneksi hardware
├── PLATFORMIO_GUIDE.md              ← PlatformIO setup guide
└── SECONDARY_SERIAL.md              ← Secondary serial config

Key Modules:
```

### 🏗️ Arsitektur Modular (OOP)

| Module | Responsibility |
|--------|----------------|
| **DisplayManager** | TFT init, primitives, fonts, color palette |
| **ECUData** | Data structures, validation flags, thresholds |
| **SpeeduinoParser** | Parse binary protocol from Speeduino |
| **SyncManager** | State machine logic, threshold checks, timeouts |
| **UIScreen** | All rendering: six-pack grid, color-coded values |
| **UIStateMachine** | Display states, dirty-flag optimization |

**Font System:**
- **UNO**: Built-in bitmap font (USE_FREEFONT = 0)
- **Mega**: FreeSansBold 18pt/12pt/9pt (USE_FREEFONT = 1)
- Conditional compilation via `__AVR_ATmega2560__`

### 🎬 Demo Test

Test suite dengan animated demo untuk verifikasi visual:

```bash
# Run animated demo (40 detik):
pio test -e uno
```

**Demo Sequence:**
1. **Cycle 1** (idle→cruise): RPM 900→2400, NORMAL state, GREEN values
2. **Cycle 2** (heavy load): RPM 2400→6400, CLT 85→113°C, AFR 13.9→11.9, WARNING state, RED values
3. **Cycle 3** (decel→recovery): RPM 6400→1150, cooldown, AFR recovery, RED→AMBER→GREEN
4. **Cycle 4** (idle fluctuation): RPM oscillation ±100, stable values, GREEN
5. **SYNC_LOSS**: 10× blink cycles, recovery progress 0%→100%

Test ini memverifikasi:
- ✅ Color transitions work correctly
- ✅ State machine responds to thresholds
- ✅ SYNC_LOSS blink alert visible
- ✅ Recovery sequence smooth
- ✅ No memory leaks or crashes
| **SyncManager** | Detect connection loss & recovery |
| **DisplayManager** | Control TFT hardware operations |
| **UIScreen** | Render UI elements & handle layout |
| **UIStateMachine** | Manage display states & redraw optimization |

---

## 🐛 Troubleshooting

### ❌ Display Not Turning On (Black Screen)

**Check 1: Power Supply**
```
→ Verify Arduino & TFT both powered (5V, sufficient current)
→ Check USB cable quality (data + power)
→ Try external 5V power supply if USB not enough
```

**Check 2: TFT Display Wiring**
```
→ Verify all 16 data lines (A0-A7, D0-D7) connected properly
→ Check control lines (CS, CD, WR, RD, RST)
→ Reseat all connectors firmly
→ Test with TFT example sketch first
```

**Check 3: Upload Configuration**
```
→ Open Device Manager → Ports → Check COM port number
→ Update platformio.ini with correct port
→ Verify board selection (uno vs megaatmega2560)
→ Rebuild & re-upload
```

### ⚠️ Compile / Build Errors

**Error: Program size exceeds maximum (UNO only)**
```
→ UNO has limited 32KB Flash (89% usage is near limit)
→ Solution: Upgrade to Mega 2560 (248KB Flash available)
→ Or: Disable debug prints, reduce features
```

**Error: MCUFRIEND_kbv not found**
```bash
pio lib install "prenticedavid/MCUFRIEND_kbv@^3.1.0-Beta"
```

**Error: Fonts not found (Mega only)**
```
→ Verify Adafruit GFX Library installed
→ FreeFonts included in Adafruit_GFX
→ Clean & rebuild: pio run --target clean
```

### 📊 Data Not Updating / Stuck Values

**Check 1: ECU Connection**
```
→ Verify Speeduino powered & running
→ Open Serial Monitor (115200 baud)
→ Check if binary data arriving
→ Verify RX1 connected to Speeduino TX
```

**Check 2: Display Shows "WAIT ECU" or "NO DATA"**
```
→ This is NORMAL if ECU not sending data yet
→ Start engine or trigger ECU output
→ Check Secondary Serial config in TunerStudio
→ Format: "Binary" or "Generic Fixed"
```

**Check 3: Display Stuck in "SYNCING"**
```
→ Data arriving but not valid/stable
→ Wait 2-3 seconds for stabilization
→ Check data format matches parser
→ Try power-cycle Arduino
```
→ Monitor → Check if data arriving from ECU
→ Verify ECU format (Binary or Secondary Serial Generic Fixed)
→ Check baud rate: 115200
```

**Check 2: Grounding Issues**
```
→ Verify all GND connections are solid
→ Use multiple ground points, not just one
→ Use thick wire for grounds
→ Check for oxidation on connectors
```

**Check 3: Cable Quality**
```
→ If serial cable >2m, use shielded twisted pair
→ If too much noise, try slower baud: 9600
→ Replace cable if corroded
```
3
---

## 📚 Additional Resources

- [PLATFORMIO_GUIDE.md](PLATFORMIO_GUIDE.md) - PlatformIO setup details
- [WIRING_GUIDE.md](WIRING_GUIDE.md) - Detailed wiring with diagrams
- [SECONDARY_SERIAL.md](SECONDARY_SERIAL.md) - Secondary serial configuration
- [README.md](README.md) - Technical documentation (English)

---

## 🤝 Contributing

Found a bug? Have an idea? Contributions welcome!

1. Fork the repository
2. Create feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -m "Add feature"`
4. Push to branch: `git push origin feature-name`
5. Open Pull Request

---

## 📄 License

Open source project. Free to use, modify, and distribute with attribution.

---

## 💬 Questions?

- Check documentation files
- Review GitHub issues
- Post in Arduino community forums

**Enjoy monitoring your engine! 🚗💨**

---

<sub>Last Updated: December 2025 | Carvionics EFIS v3.1 - Six-Pack Avionics Edition</sub>
