# 🚗 Carvionics EFIS - Display Engine Speeduino

**Sistem display mesin bergaya cockpit pesawat untuk ECU Speeduino**

> Proyek Arduino Open Source yang memantau real-time data mesin Anda di layar TFT 320×240 dengan gaya avionics yang keren. Dirancang khusus untuk Speeduino ECU menggunakan Arduino Mega atau Uno.

---

## 📌 Daftar Isi

- [Apa itu Carvionics?](#apa-itu-carvionics)
- [Fitur Unggulan](#fitur-unggulan)
- [Hardware yang Dibutuhkan](#hardware-yang-dibutuhkan)
- [Setup & Instalasi](#setup--instalasi)
- [Cara Menggunakan](#cara-menggunakan)
- [Struktur Kode](#struktur-kode)
- [Troubleshooting](#troubleshooting)

---

## ❓ Apa itu Carvionics?

Carvionics EFIS adalah sistem display untuk memonitor kondisi mesin kendaraan Anda secara real-time. Sistem ini:

- 📡 **Hanya Membaca Data** - Tidak mengirim apapun ke ECU, aman & terpercaya
- 🎨 **Gaya Cockpit** - Tampilannya seperti instrument panel pesawat, profesional & mudah dibaca
- ⚡ **Update Cepat** - Data diperbarui 20 kali per detik (20Hz)
- 🛡️ **Aman & Stabil** - Punya sistem deteksi jika koneksi putus
- 🎯 **Mudah Dipahami** - Kode terstruktur dengan baik (OOP - Object Oriented Programming)

Dalam bahasa sederhana: **Carvionics adalah "speedometer avionics" untuk mesin Speeduino Anda!**

---

## 🌟 Fitur Unggulan

| Fitur | Deskripsi |
|-------|-----------|
| **Tampilan RPM** | Angka RPM besar di tengah layar (paling penting!) |
| **Data Mesin Inti** | Suhu mesin (CLT), AFR, MAP, Tegangan baterai dalam 1 tampilan |
| **Kontrol Data** | TPS (throttle), IAT (intake air temp), status koneksi |
| **Deteksi Status** | Otomatis mendeteksi: Normal → Warning → Data Hilang |
| **Alert Berkedip** | Layar berkedip merah jika koneksi ke mesin terputus |
| **Validasi Data** | Cek otomatis agar data yang ditampilkan benar |
| **Dual Board** | Kompatibel dengan Arduino Mega (recommended) atau Uno |

---

## 🔧 Hardware yang Dibutuhkan

### Setup Minimal

Anda membutuhkan 3 komponen utama:

```
┌─────────────────────────────────────┐
│   Arduino Mega 2560 (Recommended)   │  ← Otak dari sistem
│   atau Arduino Uno (Hemat budget)   │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│  Layar TFT 2.4" (320×240 pixel)     │  ← Untuk tampilan data
│  Tipe: ILI9325 atau ILI9341         │
└──────────────┬──────────────────────┘
               │
┌──────────────┴──────────────────────┐
│  Speeduino ECU (dengan serial out)  │  ← Pengirim data mesin
│  Connected via UART Serial          │
└─────────────────────────────────────┘
```

### Spesifikasi Komponen

| Komponen | Spesifikasi | Catatan |
|----------|-------------|---------|
| **Microcontroller** | ATmega2560 16MHz | Mega: 8KB RAM (best), Uno: 2KB RAM (hemat) |
| **Layar TFT** | 2.4" 320×240 pixels | Konektor parallel 16-pin |
| **Kecepatan Serial** | 115200 baud, 8N1 | Standar Speeduino |
| **Power Supply** | 7-12V external atau USB | Layar butuh 5V regulated |
| **Penggunaan RAM** | ~1.4 KB (Mega), ~1.2 KB (Uno) | Efisien, banyak space tersisa |
| **Penggunaan Flash** | ~16 KB | Cukup untuk custom logic |

### Wiring (Arduino Mega - Yang Paling Mudah)

```
┌────────────────────────────────────────────────┐
│  TFT Display 2.4" Pins → Arduino Mega Pins     │
├────────────────────────────────────────────────┤
│  Data Lines (8 kabel):                         │
│    D0 → A0    |  D4 → A4                       │
│    D1 → A1    |  D5 → A5                       │
│    D2 → A2    |  D6 → A6                       │
│    D3 → A3    |  D7 → A7                       │
├────────────────────────────────────────────────┤
│  Control Lines (5 kabel):                      │
│    CS  → D3                                    │
│    CD  → D2                                    │
│    WR  → D4                                    │
│    RD  → D5                                    │
│    RST → D6                                    │
├────────────────────────────────────────────────┤
│  Power:                                        │
│    VCC → 5V (via regulator)                    │
│    GND → GND (common ground)                   │
├────────────────────────────────────────────────┤
│  Speeduino Serial:                             │
│    TX (ECU) → RX1 (pin 19 di Mega)             │
│    GND (ECU) → GND (common ground)             │
└────────────────────────────────────────────────┘
```

> 💡 **Tips**: Gunakan kabel berkualitas baik supaya koneksi data stabil. Grounding yang bagus sangat penting!

---

## 🚀 Setup & Instalasi

### 1️⃣ Install Tools

Anda perlu install **PlatformIO** (IDE berbasis VSCode yang mudah untuk Arduino):

**Opsi A: Langsung dari VSCode (Recommended)**
- Install extension "PlatformIO IDE" dari VSCode marketplace
- Restart VSCode
- PlatformIO akan auto-install tools yang diperlukan

**Opsi B: Manual (Jika belum punya Python)**
- Install [Python 3.8+](https://www.python.org)
- Install [VSCode](https://code.visualstudio.com)
- Install PlatformIO via terminal: `pip install platformio`

### 2️⃣ Download/Clone Project

```bash
git clone https://github.com/YourUsername/Carvionics.git
cd Carvionics
```

Atau jika copy-paste folder, pastikan struktur folder sesuai workspace di atas.

### 3️⃣ Konfigurasi Hardware (Penting!)

Edit file `platformio.ini` sesuai board Anda:

**Untuk Arduino Mega** (recommended):
```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
upload_port = COM3        ; ← Sesuaikan dengan port Anda
monitor_port = COM3       ; ← Sama
upload_speed = 115200
```

**Untuk Arduino Uno** (jika budget terbatas):
```ini
[env:uno]
platform = atmelavr
board = uno
upload_port = COM5        ; ← Sesuaikan dengan port Anda
monitor_port = COM5       ; ← Sama
upload_speed = 115200
```

### 4️⃣ Install Libraries

PlatformIO akan otomatis download library yang diperlukan saat build pertama kali:
- **MCUFRIEND_kbv** - Driver untuk TFT display
- **Adafruit GFX** - Library grafis dasar
- **Arduino Core** - Standard Arduino library

### 5️⃣ Compile & Upload

**Via VSCode PlatformIO (Paling Mudah):**

1. Buka Command Palette: `Ctrl + Shift + P`
2. Ketik `PlatformIO: Build` untuk compile
3. Jika sukses, ketik `PlatformIO: Upload` untuk upload ke board

**Via Terminal (Jika prefer CLI):**

```bash
# Untuk Mega:
pio run -e megaatmega2560 --target upload

# Untuk Uno:
pio run -e uno --target upload
```

✅ **Selesai!** Layar Anda akan hidup dan menampilkan data mesin.

---

## 💻 Cara Menggunakan

### Status Layar

Saat program pertama kali jalan, Anda akan melihat beberapa state:

| Status | Tampilan | Artinya |
|--------|----------|---------|
| **NO DATA** | Layar hitam kosong | Menunggu koneksi dari ECU Speeduino |
| **NORMAL** | Tampilan normal, warna hijau | Data mesin normal, semua aman ✅ |
| **CAUTION** | Warna kuning/amber | Ada parameter yang mulai out of range ⚠️ |
| **WARNING** | Warna merah | Ada parameter yang critical 🔴 |
| **SYNC LOSS** | Layar kedip merah | Koneksi putus, perlu dicek kabel ❌ |

### Tombol & Interaksi

Saat ini versi ini adalah **passive display** - hanya menampilkan data tanpa button interaksi. Navigasi otomatis berdasarkan state mesin.

Fitur future:
- Tombol untuk toggle antara tampilan berbeda
- Setting threshold alarm custom

---

## 📂 Struktur Kode

```
Carvionics/
│
├── src/                           ← Kode utama
│   ├── main.cpp                   ← Bagian awal program (setup & loop)
│   └── lib/                       ← Implementasi class
│       ├── ECUData.cpp            ← Penyimpan data mesin
│       ├── SpeeduinoParser.cpp    ← Pembaca serial dari ECU
│       ├── SyncManager.cpp        ← Deteksi koneksi putus
│       ├── DisplayManager.cpp     ← Driver untuk TFT
│       ├── UIScreen.cpp           ← Fungsi gambar di layar
│       └── UIStateMachine.cpp     ← Logic untuk state mesin
│
├── include/                       ← Header files (definisi class)
│   ├── ECUData.h
│   ├── SpeeduinoParser.h
│   ├── SyncManager.h
│   ├── DisplayManager.h
│   ├── UIScreen.h
│   └── UIStateMachine.h
│
├── platformio.ini                 ← Konfigurasi build
├── README.md                      ← Dokumentasi detil (English)
└── README_ID.md                   ← Dokumentasi ini (Bahasa Indonesia)
```

### Class Utama (Penjelasan Singkat)

**1. ECUData** 
```
Tugasnya: Simpan data mesin (RPM, temperatur, dll)
Analogi: "Buku catatan mesin"
```

**2. SpeeduinoParser**
```
Tugasnya: Baca data dari serial (kabel dari ECU)
Analogi: "Telinga yang mendengarkan ECU"
```

**3. SyncManager**
```
Tugasnya: Cek apakah data masih berdatangan atau koneksi putus
Analogi: "Perawat yang cek detak jantung pasien"
```

**4. DisplayManager**
```
Tugasnya: Kontrol layar TFT (gambar, warna, refresh)
Analogi: "Tangan yang menggambar di layar"
```

**5. UIScreen**
```
Tugasnya: Hitung posisi & warna elemen UI
Analogi: "Desainer yang atur layout tampilan"
```

**6. UIStateMachine**
```
Tugasnya: Atur state (Normal/Warning/SyncLoss) & apa yang perlu digambar ulang
Analogi: "Manager yang koordinasi semua state"
```

---

## 🐛 Troubleshooting

### ❌ "Layar Tidak Hidup / Hitam Polos"

**Kemungkinan 1: Port serial salah**
- Cek di Device Manager (Windows) → Ports → Lihat COM port Arduino Anda
- Update `platformio.ini` dengan port yang benar
- Compile & upload ulang

**Kemungkinan 2: Kabel TFT lepas atau salah**
- Cabut semua kabel data TFT (16 kabel A0-A7, CS, CD, WR, RD, RST)
- Cek di display datasheet kalau ada yang terbalik
- Ulangi pemasangan dengan teliti
- Coba lagi

**Kemungkinan 3: Speeduino belum kirim data**
- Pastikan ECU Speeduino sudah nyala & berjalan
- Cek dengan Serial Monitor (PlatformIO) apakah data masuk
- Update `SERIAL_BAUD` di `platformio.ini` kalau baud rate berbeda

### ⚠️ "Compile Error / Build Gagal"

**Error: undefined reference to ...**
- Pastikan semua file di `src/lib/` ada
- Cek `platformio.ini` sudah include library yang benar
- Rebuild: `PlatformIO: Clean` lalu `PlatformIO: Build`

**Error: MCUFRIEND_kbv not found**
- Library belum download
- Open terminal & jalankan: `pio lib install "prenticedavid/MCUFRIEND_kbv@^3.1.0-Beta"`

### 🔌 "Data Hilang / Banyak Error"

**Kemungkinan: Grounding jelek**
- Pastikan GND (negatif) ECU, Arduino, dan TFT **terhubung langsung satu sama lain**
- Jangan cuma satu titik - buat multiple ground connection
- Gunakan kabel tebal untuk ground

**Kemungkinan 2: Kabel serial long/berjauhan**
- Kalau kabel RX lebih dari 1-2 meter, gunakan twisted pair untuk TX-GND
- Kurangi kecepatan baud jika banyak noise: ubah `115200` ke `9600` di `platformio.ini`

### 📊 "Angka Tidak Update / Tertanam"

- Cek Serial Monitor - apakah data masuk dari ECU?
- Pastikan ECU format "Speeduino Binary" atau "Secondary Serial Generic Fixed"
- Check `SyncManager` untuk deteksi data timeout
- Debug dengan menambah `Serial.print()` di `main.cpp`

---

## 📚 Bacaan Lebih Lanjut

- [PLATFORMIO_GUIDE.md](PLATFORMIO_GUIDE.md) - Setup detail untuk PlatformIO
- [WIRING_GUIDE.md](WIRING_GUIDE.md) - Panduan wiring lengkap dengan gambar
- [SECONDARY_SERIAL.md](SECONDARY_SERIAL.md) - Konfigurasi secondary serial
- [README.md](README.md) - Dokumentasi teknis (English)

---

## 🤝 Kontribusi

Punya ide? Mau improve? Sangat welcome! 

- Fork repository
- Buat branch baru: `git checkout -b fitur-baru`
- Commit changes: `git commit -m "Tambah fitur X"`
- Push: `git push origin fitur-baru`
- Buat Pull Request

---

## 📄 Lisensi

Proyek ini open source. Bebas digunakan, modify, dan distribute dengan tetap menyebutkan original creator.

---

## 💬 Pertanyaan?

- Baca FAQ di dokumentasi teknis
- Check issue di repository
- Tanya di forum Arduino Indonesia

**Happy Monitoring! 🚗💨**

---

<sub>Last Updated: Desember 2025 | Carvionics EFIS v3.0</sub>
