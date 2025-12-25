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
| **Primary RPM Display** | Angka RPM besar & dominan di tengah layar |
| **Engine Core Data** | CLT, AFR, MAP, Battery dalam 4-quadrant layout |
| **Control Parameters** | TPS, IAT, Frame counter, Sync status indicator |
| **State Machine** | NO_DATA → NORMAL → CAUTION → WARNING → SYNC_LOSS → RECOVERY |
| **Visual Alert** | Full-screen red blink saat sync loss terdeteksi |
| **Data Validation** | Timeout detection, threshold checks, checksum verify |
| **Dirty Flag Optimization** | Per-slice redraw untuk anti-flicker & smooth animation |

---

## 📱 Mockup Tampilan

### Status: NORMAL (Green - All Systems GO)
```
╔══════════════════════════════════════════════╗
║                                              ║
║                                              ║
║              ┌─────────────────┐             ║
║              │      5850       │             ║
║              │      RPM        │             ║
║              └─────────────────┘             ║
║                                              ║
║         ┌──────────┬──────────┐              ║
║         │   CLT    │   AFR    │              ║
║         │  85°C    │  13.2:1  │              ║
║         ├──────────┼──────────┤              ║
║         │   MAP    │   BAT    │              ║
║         │ 650 kPa  │ 13.8V    │              ║
║         └──────────┴──────────┘              ║
║                                              ║
║  TPS: 25%  IAT: 32°C  SYNC: OK ✓            ║
║                                              ║
║           STATUS: ✅ NORMAL                  ║
║                                              ║
╚══════════════════════════════════════════════╝
        320 x 240 pixels (2.4" TFT)
        Background Color: GREEN
```

### Status: CAUTION (Yellow - Attention Required)
```
╔══════════════════════════════════════════════╗
║                                              ║
║                 ⚠️ CAUTION ⚠️                ║
║                                              ║
║              ┌─────────────────┐             ║
║              │      5850       │             ║
║              │      RPM        │             ║
║              └─────────────────┘             ║
║                                              ║
║         ┌──────────┬──────────┐              ║
║         │   CLT    │   AFR    │              ║
║         │ ⚠️ 92°C  │  13.2:1  │              ║
║         ├──────────┼──────────┤              ║
║         │   MAP    │   BAT    │              ║
║         │ 650 kPa  │ 13.8V    │              ║
║         └──────────┴──────────┘              ║
║                                              ║
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

### 4️⃣ NORMAL OPERATION (MAIN DISPLAY)
```
┌──────────────────────────────────────────────┐
│ ECU            SYNC OK           BAT 13.9   │
├──────────────────────────────────────────────┤
│ RPM 2450        MAP  42           │
│ CLT  87         IAT  31           │
│ AFR 14.4        TPS   3           │
├──────────────────────────────────────────────┤
│ ADV  18         DWL 2.8           │
│ ISC  42                             │
├──────────────────────────────────────────────┤
│ FUEL 0.38 L                          │
└──────────────────────────────────────────────┘
```

- Semua ECU truth
- Font besar & seimbang
- Tidak redundan dengan dashboard analog

### 5️⃣ CAUTION STATE

Contoh: CLT tinggi / AFR lean

```
┌──────────────────────────────────────────────┐
│ ECU            SYNC OK           BAT 13.4   │
├──────────────────────────────────────────────┤
│ RPM 2450        MAP  42           │
│ CLT  97 !       IAT  31           │
│ AFR 16.2 !      TPS   3           │
├──────────────────────────────────────────────┤
│ ADV  18         DWL 2.8           │
│ ISC  48                             │
├──────────────────────────────────────────────┤
│ CAUTION                                   │
└──────────────────────────────────────────────┘
```

- Angka tetap tampil
- Warna bicara, bukan ukuran
- Footer satu kata (avionic rule)

### 6️⃣ WARNING STATE

Contoh: Overheat / battery drop

```
┌──────────────────────────────────────────────┐
│ ECU            SYNC OK           BAT 11.8   │
├──────────────────────────────────────────────┤
│ RPM 2600        MAP  55           │
│ CLT 108 !!      IAT  38           │
│ AFR 15.9 !      TPS  12           │
├──────────────────────────────────────────────┤
│ ADV  10         DWL 2.6           │
│ ISC  65                             │
├──────────────────────────────────────────────┤
│ WARNING                                   │
└──────────────────────────────────────────────┘
```

- Tetap readable
- Tidak panik visual
- Fokus ke angka penting

### 7️⃣ SYNC LOSS (FULL OVERRIDE)
```
┌──────────────────────────────────────────────┐
│                                              │
│            ⚠  SYNC LOSS  ⚠                  │
│                                              │
│        CHECK TRIGGER / CAM / CRANK           │
│                                              │
│         RPM / IGN INVALID                    │
│                                              │
└──────────────────────────────────────────────┘
```

- Override total
- Tidak ada data lain
- Ini avionic non-negotiable

### 8️⃣ RECOVERY (SETELAH SYNC BALIK)
```
┌──────────────────────────────────────────────┐
│ ECU            RECOVERING...                 │
├──────────────────────────────────────────────┤
│ RPM ----        MAP --                       │
│ CLT --          IAT --                       │
│ AFR --          TPS --                       │
└──────────────────────────────────────────────┘
```

- Delay singkat
- Hindari flicker & false alarm

### 🧭 Ringkasan State Machine

```
BOOT
 ↓
WAIT_ECU
 ↓
SYNCING
 ↓
NORMAL
        ↳ CAUTION
        ↳ WARNING
        ↳ SYNC_LOSS → RECOVERY → NORMAL
```

## 🔧 Hardware yang Dibutuhkan

### 📦 Komponen Utama

Anda membutuhkan 3 komponen inti:

```
┌─────────────────────────────────────┐
│  Arduino Mega 2560 or Uno           │  ← Microcontroller
│  (Mega recommended, Uno budget)     │
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
1. Buka Command Palette: `Ctrl + Shift + P`
2. `PlatformIO: Build` → compile
3. `PlatformIO: Upload` → upload ke board

**Via Terminal:**
```bash
# Mega:
pio run -e megaatmega2560 --target upload

# Uno:
pio run -e uno --target upload
```

✅ **Done!** Layar akan hidup dan menampilkan data mesin.

---

## 💻 Cara Menggunakan

### 🎯 Display States

Carvionics memiliki 5 state utama yang ditunjukkan dengan warna & tampilan berbeda:

| State | Warna | Arti | Action |
|-------|-------|------|--------|
| **NO_DATA** | Black | Menunggu koneksi ECU | Check kabel serial |
| **NORMAL** | Green | Semua parameter normal ✅ | Monitor biasa |
| **CAUTION** | Yellow | Ada parameter warning ⚠️ | Perhatian diperlukan |
| **WARNING** | Red | Ada parameter critical 🔴 | Tindakan segera! |
| **SYNC_LOSS** | Red Blink | Koneksi terputus ❌ | Check physical connection |

### 🎮 Tombol & Interaksi

Saat ini adalah **passive display** - hanya menampilkan data tanpa button interaksi.

Future features:
- Button untuk toggle display modes
- Custom threshold settings
- Data logging

---

## 📂 Struktur Kode

```
Carvionics/
├── src/
│   ├── main.cpp                     ← Entry point (setup & main loop)
│   └── lib/
│       ├── ECUData.cpp              ← Data container & validation
│       ├── SpeeduinoParser.cpp      ← Serial frame decoder
│       ├── SyncManager.cpp          ← Sync loss detection state machine
│       ├── DisplayManager.cpp       ← TFT driver wrapper
│       ├── UIScreen.cpp             ← UI rendering logic
│       └── UIStateMachine.cpp       ← State orchestration
│
├── include/
│   ├── ECUData.h
│   ├── SpeeduinoParser.h
│   ├── SyncManager.h
│   ├── DisplayManager.h
│   ├── UIScreen.h
│   └── UIStateMachine.h
│
└── platformio.ini                   ← Build configuration
```

### 🔍 Class Descriptions

| Class | Purpose |
|-------|---------|
| **ECUData** | Store & validate engine parameters |
| **SpeeduinoParser** | Decode binary serial frames from ECU |
| **SyncManager** | Detect connection loss & recovery |
| **DisplayManager** | Control TFT hardware operations |
| **UIScreen** | Render UI elements & handle layout |
| **UIStateMachine** | Manage display states & redraw optimization |

---

## 🐛 Troubleshooting

### ❌ Display Not Turning On (Black Screen)

**Check 1: Serial Port Configuration**
```
→ Open Device Manager → Ports → Check COM port number
→ Update platformio.ini with correct port
→ Rebuild & re-upload
```

**Check 2: TFT Display Wiring**
```
→ Verify all 16 data lines (A0-A7, D0-D7) connected properly
→ Check control lines (CS, CD, WR, RD, RST)
→ Reseat all connectors firmly
→ Retry
```

**Check 3: Speeduino Sending Data**
```
→ Verify ECU is powered & running
→ Open Serial Monitor to verify data arrival
→ Check baud rate matches (115200)
→ Verify UART cable connected to RX1
```

### ⚠️ Compile / Build Errors

**Error: MCUFRIEND_kbv not found**
```bash
pio lib install "prenticedavid/MCUFRIEND_kbv@^3.1.0-Beta"
```

**Error: undefined reference**
```
→ Verify all .cpp files in src/lib/ present
→ Run: pio run --target clean
→ Rebuild
```

### 📊 Data Not Updating / Stuck Values

**Check 1: Serial Connection**
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

<sub>Last Updated: December 2025 | Carvionics EFIS v3.0</sub>
