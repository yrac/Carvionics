# Secondary Serial (Generic Fixed) — Quick Guide

Dokumen ini menjelaskan cara mengaktifkan dan menggunakan output Secondary Serial “Generic Fixed” (CSV) dari Speeduino dengan firmware Carvionics EFIS.

Tujuan:
- Pasif (read-only), tidak mengirim perintah ke ECU
- Parser otomatis mendeteksi Binary (0xAA) atau CSV Generic Fixed
- Hanya field yang tersedia akan ditampilkan; yang tidak ada akan “—”

---

## 1) Mengaktifkan Secondary Serial di TunerStudio

1. Buka TunerStudio untuk proyek Speeduino kamu.
2. Enable Secondary Serial IO Interface (menu project settings → Secondary IO).
3. Pilih mode “Generic Fixed” (simple/enhanced sesuai kebutuhan).
4. Baud rate: 115200, format: 8N1.
5. Pastikan ECU mengirim satu baris (CSV) per update, diakhiri newline (\n/\r\n).

Catatan: Pada Arduino Mega 2560, Speeduino biasanya memakai Serial3 untuk Secondary IO. Cek konfigurasi Speeduino kamu dan sesuaikan wiring/port di firmware bila perlu.

---

## 2) Wiring & Port Serial di Firmware

- Mega 2560 (default firmware): Dengarkan di Serial1 (RX1 / Pin 19)
  - ECU TX → Mega RX1 (Pin 19)
  - ECU GND → GND
- Opsi Mega 2560 (jika Secondary IO di Serial3):
  - ECU TX → Mega RX3 (Pin 15)
  - Ubah inisialisasi parser di `src/main.cpp`:

```cpp
#ifdef ARDUINO_AVR_MEGA2560
  // Default:
  // parser.begin(Serial1);
  // Jika ECU mengirim CSV di Serial3, ganti menjadi:
  parser.begin(Serial3);
#else
  parser.begin(Serial);
#endif
```

- Uno: Memakai Serial (shared dengan Serial Monitor). Matikan monitor saat pengujian jika data bertabrakan.

---

## 3) Urutan Field Generic Fixed & Skala Nilai

Parser mengasumsikan urutan field tetap yang umum berikut. Jika konfigurasi ECU berbeda, mapping bisa disesuaikan di parser tanpa mengubah API publik.

Urutan umum:
1. RPM (integer, RPM)
2. MAP (integer, kPa)
3. TPS (integer, %)
4. CLT (integer, °C)
5. IAT (integer, °C)
6. AFR (float, mis. 14.7) atau 100x (mis. 1470)
7. BAT (float volt, mis. 12.6) atau 0.1V (mis. 123 → 12.3V)

Aturan skala:
- AFR: jika < 50 → dianggap float (14.7 → 1470 internal 100x); jika > 50 → dianggap sudah 100x.
- Battery: 0..30 → volt (×1000 jadi mV); 0..160 → 0.1V (×100 jadi mV); <30000 → dianggap mV.

Hanya field yang tersedia di baris CSV yang akan di-update; field yang tidak ada dibiarkan, sehingga UI menampilkan “—”. Tidak ada estimasi atau perhitungan tambahan.

---

## 4) Contoh CSV

Contoh baris Generic Fixed (1 baris per update):

```
3500,45,18,92,30,14.7,12.6
```

Arti:
- RPM=3500, MAP=45 kPa, TPS=18%, CLT=92°C, IAT=30°C, AFR=14.7, Battery=12.6V

Contoh dengan sebagian field (subset) saja:

```
2800,52,,,,,12.3
```

Arti:
- RPM=2800, MAP=52 kPa, Battery=12.3V, field lain kosong → UI akan menampilkan “—” untuk field tersebut.

---

## 5) Perilaku UI

- Angka utama selalu putih (ECU truth). Warna hanya dipakai pada border/header sebagai status.
- SYNC LOSS tetap override fullscreen (blink).
- CLT/AFR memiliki anotasi (!) atau (!!) berdasar threshold `SyncManager`.
- Field yang tidak tersedia akan tampil “—”.

---

## 6) Troubleshooting

- Tidak ada update data:
  - Pastikan port serial benar (Serial1 vs Serial3 di Mega).
  - Cek bahwa Speeduino benar-benar mengirim CSV (baris diakhiri newline).
  - Baud rate di ECU dan firmware sama (115200).
  - Gunakan `parser.debugPrint()` di Serial Monitor untuk melihat statistik parser.

- Blink SYNC LOSS terus:
  - Pastikan interval kirim CSV stabil (tanpa jeda panjang).
  - Cek threshold dan timeout di `SyncManager`.

- Format angka tidak sesuai:
  - Periksa skala AFR/Battery yang dikirim ECU; sesuaikan jika perlu.

---

## 7) Penyesuaian Mapping (Opsional)

Jika urutan field ECU berbeda, mapping bisa dikunci di `SpeeduinoParser` dengan mengganti indeks internal (tanpa mengubah API publik). Arahkan aku ke urutan field yang dipakai, atau kirim satu contoh baris CSV, agar mapping bisa dipastikan tepat.

---

Last Updated: 26 December 2025
