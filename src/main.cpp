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
// GLOBAL OBJECTS
// ============================================================================

ECUData ecu_data;                           // Engine data container
SpeeduinoParser parser(115200);             // Speeduino serial parser
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
const uint32_t RENDER_INTERVAL_MS = 50;    // 20 Hz update rate
const uint32_t DEBUG_INTERVAL_MS = 5000;   // Debug every 5 seconds

// ============================================================================
// ARDUINO SETUP
// ============================================================================

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
    Serial.print("Initializing Speeduino parser...");
    #ifdef ARDUINO_AVR_MEGA2560
    parser.begin(Serial1);  // Use Serial1 for Speeduino on Mega
    #else
    parser.begin(Serial);   // Use Serial for Speeduino on Uno (shared with Serial Monitor)
    #endif
    Serial.println(" OK");
    
    // Set default thresholds
    SyncManager::Thresholds thresholds;
    thresholds.rpm_max = 8000;
    thresholds.clt_min = 10;
    thresholds.clt_max = 110;
    thresholds.afr_min = 1200;
    thresholds.afr_max = 1700;
    thresholds.battery_min = 11000;
    thresholds.data_timeout_ms = 500;
    thresholds.recovery_delay_ms = 2000;
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
