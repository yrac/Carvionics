# DISPLAY PUTIH - TROUBLESHOOTING GUIDE

**Masalah**: Display menunjukkan layar putih (tidak ada gambar)

## Penyebab Utama

### 1. **MCUFRIEND_kbv Library Tidak Terinstall** ⚠️ PALING SERING
Status: **SEKARANG TERJADI PADA SISTEM ANDA**

Kode menggunakan fallback mock class yang kosong - tidak mengirim apapun ke display!

**Solusi:**
```bash
# Via PlatformIO CLI
platformio lib install "MCUFRIEND_kbv"

# Via platformio.ini
lib_deps = 
    MCUFRIEND_kbv
    Adafruit GFX Library
```

### 2. **Pin Wiring Salah**
Jika display tidak berespons sama sekali

Periksa koneksi (Arduino Mega 2560):
```
MEGA PIN → DISPLAY PIN
Port A (D22-D29) → DB0-DB7 (Data)
A0 (D14) → RD (Read Enable)
A1 (D15) → WR (Write Enable)  
A2 (D16) → RS (Register Select / DC)
A3 (D17) → CS (Chip Select)
A4 (D18) → RST (Reset)

5V → VCC
GND → GND
```

### 3. **Chip Display Tidak Dikenali**
Library MCUFRIEND hanya support chip tertentu:
- ILI9325, ILI9328, ILI9341, ILI9342
- HX8347, HX8357, ST7783
- RM68140 (limited support)

Jika chip lain → perlu library berbeda

### 4. **Display Reset Tidak Bekerja**
RST (Reset) pin tidak terkoneksi atau tidak ada high pulse di startup

### 5. **Tegangan/Power Tidak Stabil**
Display butuh arus cukup. Artefak putih = power issue

---

## Langkah-Langkah Troubleshooting

### LANGKAH 1: Cek Wiring Fisik
```
☐ Semua 16 kabel terhubung (8 data + 5 control + 2 power)?
☐ Tidak ada kabel loose/putus?
☐ Polaritas power benar (5V ke VCC, GND ke GND)?
☐ Reset pin (RST) terhubung ke D18?
```

### LANGKAH 2: Identifikasi Chip Display
Upload sketch detection:
```bash
# Salin file ke sketch untuk Arduino IDE atau
# Buat file baru di src/ untuk PlatformIO

pio run -t uploadfs --environment uno
```

Buka Serial Monitor (115200 baud):
```
Akan tercetak:
  Display ID: 0x____
  Chip: [jenis chip]
```

Catat chip ID!

### LANGKAH 3: Install Library yang Tepat

**Opsi A: Install MCUFRIEND_kbv (Recommended)**
```bash
cd d:\Solution\Carvionics
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe lib install "MCUFRIEND_kbv"
```

**Opsi B: Update platformio.ini**
```ini
[env:megaatmega2560]
platform = atmelavr
board = megaatmega2560
framework = arduino
lib_deps = 
    Adafruit GFX Library
    MCUFRIEND_kbv
monitor_speed = 115200

[env:uno]
platform = atmelavr
board = uno
framework = arduino
lib_deps = 
    Adafruit GFX Library
    MCUFRIEND_kbv
monitor_speed = 115200
```

### LANGKAH 4: Rebuild & Test
```bash
# Clean build
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e megaatmega2560 --target clean
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe run -e megaatmega2560
```

### LANGKAH 5: Monitor Serial Output
```bash
C:\Users\aryca\.platformio\penv\Scripts\platformio.exe device monitor
```

Lihat output boot sequence - harus ada pesan "TFT display... OK"

---

## Test Procedure

### Simple Color Test (Gauss)
Buat file `src/test_colors.cpp`:

```cpp
#include <Arduino.h>
#ifdef MCUFRIEND_KBV_H
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

MCUFRIEND_kbv tft;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("TFT Color Test");
    
    uint16_t ID = tft.readID();
    Serial.print("ID: 0x");
    Serial.println(ID, HEX);
    
    tft.begin(ID);
    tft.setRotation(0);
    
    // Test RED
    Serial.println("Drawing RED...");
    tft.fillScreen(0xF800);
    delay(1000);
    
    // Test GREEN
    Serial.println("Drawing GREEN...");
    tft.fillScreen(0x07E0);
    delay(1000);
    
    // Test BLUE
    Serial.println("Drawing BLUE...");
    tft.fillScreen(0x001F);
    delay(1000);
    
    // Test WHITE
    Serial.println("Drawing WHITE...");
    tft.fillScreen(0xFFFF);
    delay(1000);
    
    // Test BLACK
    Serial.println("Drawing BLACK...");
    tft.fillScreen(0x0000);
    
    Serial.println("Test complete!");
}

void loop() {
    delay(1000);
}

#else
#error "MCUFRIEND_kbv library not found!"
#endif
```

Compile & upload, lihat Serial Monitor:
```
TFT Color Test
ID: 0x9341
Drawing RED...
Drawing GREEN...
...
Test complete!
```

Jika layar berubah warna → **library terinstall dengan benar!**
Jika tetap putih → **hardware/wiring issue**

---

## Diagnosis Matrix

| Kondisi | Penyebab | Solusi |
|---------|---------|--------|
| Putih terus | MCUFRIEND belum install | `pio lib install MCUFRIEND_kbv` |
| Putih + error di compile | Library not found | Add to platformio.ini lib_deps |
| Warna berubah di test | Bekerja! (lanjut main app) | Upload main.cpp |
| Warna tidak berubah | Wiring salah | Periksa pin connections |
| Wiring OK + tetap putih | Chip tidak didukung | Identifikasi dengan detection sketch |
| No output sama sekali | Power/Reset issue | Check RST pin, 5V supply |

---

## Pin Reference (Arduino Mega 2560)

```
MCUFRIEND SHIELD PINOUT:
┌─────────────────────────────────────┐
│ GND  VCC  5V  GND                   │
│ D0(22) D1(23) D2(24) D3(25)         │ Data pins
│ D4(26) D5(27) D6(28) D7(29)         │ (Port A)
│ RD(14) WR(15) RS(16) CS(17) RST(18) │ Control pins
└─────────────────────────────────────┘

Arduino Mega Socket:
- D0-D7 = PORTD (atau Port A untuk beberapa board)
- A0-A15 = Port F

Untuk MCUFRIEND standard:
- Data: D22-D29 (Port A)
- RD: D14 (PA1)
- WR: D15 (PA2)
- RS: D16 (PA3) 
- CS: D17 (PA4)
- RST: D18 (PA5)
```

---

## Quick Checklist

**Sebelum Report Issue:**
- [ ] Cek physical wiring 3x
- [ ] Jalankan detection sketch
- [ ] Install MCUFRIEND_kbv via lib install
- [ ] Run color test sketch  
- [ ] Check serial output untuk errors
- [ ] Clean build & upload ulang
- [ ] Monitor serial 30 detik

**Jika masih error, provide:**
```
1. Output detection sketch (chip ID)
2. Serial monitor output saat boot
3. Error message dari compile (jika ada)
4. Board: Mega atau Uno?
5. Display type/model label
6. Photo wiring (jika bisa)
```

---

## Contact & Support

Jika masih stuck, siapkan:
- Chip ID dari detection sketch
- Foto/diagram wiring
- Serial monitor full output
- Exact error message

Kemudian akan buat custom library adapter untuk chip Anda!
