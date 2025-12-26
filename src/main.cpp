/**
 * Carvionics EFIS - Speeduino Avionics Display
 * 
 * Aircraft cockpit-style passive engine display
 * PlatformIO + Arduino Mega 2560 / Arduino Uno
 * 
 * Display: MCUFRIEND_kbv TFT 320x240 (8-bit parallel)
 * Communication: Serial read-only (Speeduino binary frame)
 * 
 * Architecture:
 * - ECUData: Data storage & status
 * - SpeeduinoParser: Serial frame decode
 * - SyncManager: State machine & thresholds
 * - UIStateMachine: Rendering orchestration
 * - DisplayManager: TFT driver wrapper
 * - UIScreen: UI rendering logic
 */

#include <Arduino.h>
#include <stdint.h>

// Include all OOP classes
#include "ECUData.h"
#include "SpeeduinoParser.h"
#include "SyncManager.h"
#include "DisplayManager.h"
#include "UIStateMachine.h"
#include "UIScreen.h"

// ============================================================================
// PRIMARY 'A' DEBUG MODE (request 'A' and parse offsets)
// ============================================================================
#ifdef DEBUG_SPEEDUINO_PRIMARY_A
// Offsets per Speeduino primary realtime response
#define OFFSET_SECL     0
#define OFFSET_STATUS1  1
#define OFFSET_ENGINE   2
#define OFFSET_MAP      4  // 2 bytes
#define OFFSET_IAT      6
#define OFFSET_CLT      7
#define OFFSET_BATTERY  9
#define OFFSET_AFR      10
#define OFFSET_RPM      14 // 2 bytes
#define OFFSET_TPS      24

int rpm;
int tps;
int map_val;
int clt;
int iat;
float afr;
float bat;

#ifndef DEBUG_REQ_CMD
#define DEBUG_REQ_CMD 'A'
#endif

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial.println("Dashboard Speeduino Siap...");
    Serial.println("Menunggu data...");
    delay(1000);
}

void requestData() {
    while (Serial1.available() > 0) { Serial1.read(); }
    Serial1.write((uint8_t)DEBUG_REQ_CMD);
    #ifdef PRIMARY_REQ_APPEND_NL
    Serial1.write('\n');
    #endif
}

void printDebug() {
    Serial.print("RPM: "); Serial.print(rpm);
    Serial.print(" | TPS: "); Serial.print(tps); Serial.print("%");
    Serial.print(" | MAP: "); Serial.print(map_val); Serial.print("kPa");
    Serial.print(" | AFR: "); Serial.print(afr);
    Serial.print(" | CLT: "); Serial.print(clt); Serial.print("C");
    Serial.print(" | IAT: "); Serial.print(iat); Serial.print("C");
    Serial.print(" | BAT: "); Serial.print(bat); Serial.println("V");
}

static inline bool isAsciiPrintable(uint8_t b) { return (b >= 32 && b <= 126) || b == '\t' || b == '\r' || b == '\n'; }

void parsePrimaryBuffer(const uint8_t *buffer, int len) {
    if (len < 30) return;
    rpm = buffer[OFFSET_RPM] + (buffer[OFFSET_RPM + 1] << 8);
    tps = buffer[OFFSET_TPS] / 2; // adjust if already percent
    map_val = buffer[OFFSET_MAP] + (buffer[OFFSET_MAP + 1] << 8);
    clt = (int)buffer[OFFSET_CLT] - 40;
    iat = (int)buffer[OFFSET_IAT] - 40;
    afr = buffer[OFFSET_AFR] / 10.0f;
    bat = buffer[OFFSET_BATTERY] / 10.0f;
}

int32_t toInt(const char *s) {
    bool neg = false; int32_t v = 0; while (*s == ' ' || *s == '\t') s++; if (*s == '-') { neg = true; s++; }
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    return neg ? -v : v;
}
float toFloat(const char *s) {
    while (*s == ' ' || *s == '\t') s++; bool neg = false; if (*s == '-') { neg = true; s++; }
    int32_t ip = 0, fp = 0, div = 1; while (*s >= '0' && *s <= '9') { ip = ip * 10 + (*s - '0'); s++; }
    if (*s == '.') { s++; while (*s >= '0' && *s <= '9') { fp = fp * 10 + (*s - '0'); div *= 10; s++; } }
    float v = (float)ip + (fp ? (float)fp / (float)div : 0.0f); return neg ? -v : v;
}

bool parseKeyValueLine(const char *line) {
    const char *p = line; bool any = false;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        const char *ks = p; while (*p && *p != '=' && *p != ',' && *p != '\n' && *p != '\r') p++;
        if (*p != '=') { while (*p && *p != ',' && *p != '\n' && *p != '\r') p++; continue; }
        const char *ke = p; p++; const char *vs = p; while (*p && *p != ',' && *p != '\n' && *p != '\r') p++;
        const char *ve = p;
        char key[12]; uint8_t ki = 0; while (ks < ke && ki < sizeof(key)-1) { char c = *ks++; if (c >= 'a' && c <= 'z') c = (char)(c-32);
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) key[ki++] = c; }
        key[ki] = '\0';
        char val[24]; uint8_t vi = 0; while (vs < ve && vi < sizeof(val)-1) { val[vi++] = *vs++; } val[vi] = '\0';
        if (strcmp(key, "RPM") == 0) { rpm = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "MAP") == 0) { map_val = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "TPS") == 0) { tps = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "CLT") == 0) { clt = (int16_t)toInt(val); any = true; }
        else if (strcmp(key, "IAT") == 0) { iat = (int16_t)toInt(val); any = true; }
        else if (strcmp(key, "AFR") == 0) { float f = toFloat(val); afr = (f > 0.0f && f < 50.0f) ? f : (float)toInt(val)/100.0f; any = true; }
        else if (strcmp(key, "BAT") == 0 || strcmp(key, "VBAT") == 0 || strcmp(key, "BATTERY") == 0) {
            float f = toFloat(val); bat = (f > 0.0f && f < 30.0f) ? f : (float)toInt(val)/10.0f; any = true; }
        if (*p == ',') p++;
    }
    return any;
}

bool parseCsvLine(const char *line) {
    // Assume order: rpm,map,tps,clt,iat,afr,bat
    const uint8_t MAXF=24; const char *fields[MAXF]; uint8_t c=0; const char *q=line; fields[c++]=q;
    while (*q && c<MAXF) { if (*q==',') fields[c++]=q+1; q++; }
    if (c<5) return false;
    auto get=[&](int i){ return (i>=0 && i<(int)c) ? fields[i] : (const char*)nullptr; };
    if (get(0)) rpm = (uint16_t)toInt(get(0));
    if (get(1)) map_val = (uint16_t)toInt(get(1));
    if (get(2)) tps = (uint16_t)toInt(get(2));
    if (get(3)) clt = (int16_t)toInt(get(3));
    if (get(4)) iat = (int16_t)toInt(get(4));
    if (get(5)) { float f = toFloat(get(5)); afr = (f>0.0f && f<50.0f) ? f : (float)toInt(get(5))/100.0f; }
    if (get(6)) { float f = toFloat(get(6)); bat = (f>0.0f && f<30.0f) ? f : (float)toInt(get(6))/10.0f; }
    return true;
}

void readAndProcess() {
    uint8_t buf[160]; int idx=0; uint32_t start=millis();
    // Read for up to 120ms or until buffer fills
    while (millis()-start < 120 && idx < (int)sizeof(buf)) {
        while (Serial1.available() && idx < (int)sizeof(buf)) {
            buf[idx++] = Serial1.read();
            // Early stop if ASCII line end
            if (buf[idx-1] == '\n' || buf[idx-1] == '\r') break;
        }
        if (idx>0 && (buf[idx-1] == '\n' || buf[idx-1] == '\r')) break;
    }

    if (idx >= 75 && !isAsciiPrintable(buf[0])) {
        parsePrimaryBuffer(buf, idx);
        printDebug();
        return;
    }
    // Try ASCII line
    bool has_eq=false, has_comma=false, ascii=true;
    for (int i=0;i<idx;i++){ if (buf[i]=='=') has_eq=true; if (buf[i]==',') has_comma=true; if (!isAsciiPrintable(buf[i])) ascii=false; }
    if (ascii && idx>0) {
        // Make null-terminated line
        char line[160]; int n = (idx < 159) ? idx : 159; for (int i=0;i<n;i++) line[i]=(char)buf[i]; line[n]='\0';
        bool ok = false;
        if (has_eq) ok = parseKeyValueLine(line); else if (has_comma) ok = parseCsvLine(line);
        if (ok) { printDebug(); return; }
    }
    // Fallback: print hex dump
    Serial.print("HEX "); Serial.print(idx); Serial.print(" bytes: ");
    for (int i=0;i<idx;i++){ if (buf[i] < 16) Serial.print('0'); Serial.print(buf[i], HEX); Serial.print(' ');} Serial.println();
}

void loop() {
    requestData();
    if (Serial1.available() >= 75) {
        readAndProcess();
    }
    delay(50);
}
#endif

// ============================================================================
// RAW DEBUG MODE (active request 'A' and hex dump)
// ============================================================================
#ifdef DEBUG_SPEEDUINO_RAW
void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    delay(1000);
    Serial.println("=== MODE DEBUG DIMULAI ===");
    Serial.println("Mengirim perintah 'A' ke Speeduino...");
}

void loop() {
    while (Serial1.available() > 0) {
        Serial1.read();
    }

    Serial1.write('A');
    delay(100);

    int jumlahData = Serial1.available();
    if (jumlahData > 0) {
        Serial.print("Diterima ");
        Serial.print(jumlahData);
        Serial.print(" bytes: ");
        while (Serial1.available() > 0) {
            byte dataMentah = Serial1.read();
            if (dataMentah < 16) Serial.print("0");
            Serial.print(dataMentah, HEX);
            Serial.print(" ");
        }
        Serial.println();
    } else {
        Serial.println("MENUNGGU... Tidak ada respon dari Speeduino (Cek Kabel RX/TX!)");
    }
    delay(500);
}
#endif

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

ECUData ecu_data;                           // Engine data container
SpeeduinoParser parser(SERIAL_BAUD);        // Speeduino serial parser
SyncManager sync_manager;                   // Sync loss & recovery state machine
DisplayManager display;                     // TFT display driver
UIStateMachine ui_state_machine;            // UI state & dirty flag management
UIScreen ui_screen(display);                // UI renderer

// ============================================================================
// SYSTEM STATE
// ============================================================================

enum class SystemState {
    BOOT,
    DISPLAY_INIT,
    PARSER_INIT,
    RUNNING,
    ERROR
};

SystemState system_state = SystemState::BOOT;
uint32_t last_render_time = 0;
uint32_t last_debug_time = 0;
const uint32_t RENDER_INTERVAL_MS = 75;    // ~13 Hz update rate (steadier)
const uint32_t DEBUG_INTERVAL_MS = 2000;   // Faster debug for link bring-up

// ============================================================================
// ARDUINO SETUP
// ============================================================================

#if !defined(UNIT_TEST) && !defined(DEBUG_SPEEDUINO_RAW) && !defined(DEBUG_SPEEDUINO_PRIMARY_A)
void setup() {
    Serial.begin(115200);
    delay(1000);  // Wait for serial monitor
    
    Serial.println("\n========================================");
    Serial.println("  CARVIONICS EFIS - BOOT SEQUENCE");
    Serial.println("========================================\n");
    
    // Initialize display
    Serial.print("Initializing TFT display...");
    if (display.begin()) {
        Serial.println(" OK");
        display.fillScreen(DisplayManager::Color::BLACK);
        // Quick visual check: draw RGB bars then clear
        display.drawTestPattern();
        delay(500);
        display.clear();
        system_state = SystemState::DISPLAY_INIT;
    } else {
        Serial.println(" FAILED");
        system_state = SystemState::ERROR;
        return;
    }
    
    // Show boot message on screen
    display.setTextSize(1);
    display.setTextColor(DisplayManager::Color::GREEN, DisplayManager::Color::BLACK);
    display.setCursor(100, 100);
    display.print("INITIALIZING...");
    
    // Initialize serial parser
    Serial.print("Initializing Speeduino parser (baud=");
    Serial.print(SERIAL_BAUD);
    Serial.println(")...");
    #ifdef ARDUINO_AVR_MEGA2560
    #ifdef USE_ECU_SERIAL3
    parser.begin(Serial3);  // Use Serial3 for Speeduino on Mega (optional)
    Serial.println("[Serial] Listening on Serial3 (RX3/PIN 15)");
    #elif defined(USE_ECU_SERIAL0)
    parser.begin(Serial);   // Use Serial0 (USB UART) for Speeduino on Mega
    Serial.println("[Serial] Listening on Serial0 (RX0/PIN 0) — may conflict with USB");
    #else
    parser.begin(Serial1);  // Use Serial1 for Speeduino on Mega
    Serial.println("[Serial] Listening on Serial1 (RX1/PIN 19)");
    #endif
    #else
    parser.begin(Serial);   // Use Serial for Speeduino on Uno (shared with Serial Monitor)
    Serial.println("[Serial] Listening on Serial (UNO RX0/PIN 0)");
    #endif
    Serial.println(" OK");

    // Primary request mode (active polling)
    #ifdef USE_PRIMARY_REQUEST
    Serial.print("[PrimaryRQ] Enabled: cmd=");
    Serial.print((char)PRIMARY_REQ_CMD);
    Serial.print(" (0x"); Serial.print((uint8_t)PRIMARY_REQ_CMD, HEX); Serial.print(") period=");
    Serial.print((uint32_t)PRIMARY_REQ_PERIOD_MS);
    Serial.println(" ms");
    parser.configureRequest((uint8_t)PRIMARY_REQ_CMD, (uint32_t)PRIMARY_REQ_PERIOD_MS);
    Serial.println("[PrimaryRQ] TX pin must connect to ECU RX; RX pin to ECU TX; GND common");
    #endif
    
    // Set default thresholds
    SyncManager::Thresholds thresholds;
    thresholds.rpm_max = 8000;
    thresholds.clt_min = 10;
    thresholds.clt_max = 110;
    thresholds.afr_min = 1200;
    thresholds.afr_max = 1700;
    thresholds.battery_min = 11000;
    thresholds.data_timeout_ms = 500;
    // Tweaked to reduce flicker/state flapping
    thresholds.data_timeout_ms = 700;
    thresholds.recovery_delay_ms = 2500;
    sync_manager.setThresholds(thresholds);
    
    system_state = SystemState::RUNNING;
    
    Serial.println("\n[SYSTEM] Boot sequence complete - RUNNING");
    Serial.println("Waiting for Speeduino data...\n");
    
    // Initialize UI state machine mengikuti state awal SyncManager (NO_DATA)
    ui_state_machine.update(sync_manager.getState());
}

// ============================================================================
// ARDUINO LOOP
// ============================================================================

void loop() {
    uint32_t now = millis();
    
    // ========== PARSER UPDATE ==========
    // Non-blocking serial read & frame parsing
    parser.update(ecu_data);
    
    // ========== SYNC MANAGER UPDATE ==========
    // Evaluate thresholds & state transitions
    sync_manager.update(ecu_data);
    
    // ========== UI STATE MACHINE UPDATE ==========
    // Orchestrate rendering (dirty flags)
    ui_state_machine.update(sync_manager.getState());
    
    // ========== RENDER DISPLAY ==========
    // Throttle rendering to avoid flicker (50ms = 20Hz)
    if (now - last_render_time >= RENDER_INTERVAL_MS) {
        last_render_time = now;
        
        // Main rendering function
        // Dirty flags prevent unnecessary redraws
        ui_screen.render(ecu_data, sync_manager, ui_state_machine);
        
        // Mark all slices as clean (for next iteration)
        // In next iteration, only changed slices will be redrawn
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::HEADER);
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::RPM_FIELD);
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::ENGINE_CORE);
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::CONTROL_DATA);
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::FOOTER);
        ui_state_machine.markSliceClean(UIStateMachine::UISlice::FULL_SCREEN);
    }
    
    // ========== DEBUG OUTPUT ==========
    // Print statistics every 5 seconds
    if (now - last_debug_time >= DEBUG_INTERVAL_MS) {
        last_debug_time = now;
        
        Serial.println("\n========== SYSTEM STATUS ==========");
        ecu_data.debugPrint();
        parser.debugPrint();
        sync_manager.debugPrint();
        Serial.println("===================================\n");
    }
    
    // Very short delay to prevent watchdog timeout
    delay(1);
}

// ============================================================================
// OPTIONAL: Serial command handler untuk testing
// ============================================================================

void handleSerialCommand() {
    if (!Serial.available()) return;
    
    char cmd = Serial.read();
    
    switch (cmd) {
        case 'd':  // Debug toggle
            Serial.println("[CMD] Debug info requested");
            ecu_data.debugPrint();
            break;
        
        case 'r':  // Reset data
            Serial.println("[CMD] Resetting ECU data");
            ecu_data.reset();
            break;
        
        case 's':  // Trigger sync loss
            Serial.println("[CMD] Triggering sync loss");
            sync_manager.triggerSyncLoss();
            break;
        
        case 'c':  // Clear screen
            Serial.println("[CMD] Clearing display");
            display.clear();
            break;
        
        case '?':  // Help
            Serial.println("\n=== COMMANDS ===");
            Serial.println("d - Debug info");
            Serial.println("r - Reset data");
            Serial.println("s - Trigger sync loss");
            Serial.println("c - Clear screen");
            Serial.println("? - This help");
            break;
        
        default:
            Serial.print("Unknown command: ");
            Serial.println(cmd);
    }
}
#endif
