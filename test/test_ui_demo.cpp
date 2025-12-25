#include <Arduino.h>
#include <unity.h>
#include "ECUData.h"
#include "SyncManager.h"
#include "DisplayManager.h"
#include "UIStateMachine.h"
#include "UIScreen.h"

static ECUData ecu;
static SyncManager syncMgr;
static DisplayManager display;
static UIStateMachine uiState;
static UIScreen screen(display);

static void markAllClean() {
    uiState.markSliceClean(UIStateMachine::UISlice::HEADER);
    uiState.markSliceClean(UIStateMachine::UISlice::RPM_FIELD);
    uiState.markSliceClean(UIStateMachine::UISlice::ENGINE_CORE);
    uiState.markSliceClean(UIStateMachine::UISlice::CONTROL_DATA);
    uiState.markSliceClean(UIStateMachine::UISlice::FOOTER);
    uiState.markSliceClean(UIStateMachine::UISlice::FULL_SCREEN);
}

static void renderTick(uint16_t ms = 50) {
    uiState.update(syncMgr.getState());
    screen.render(ecu, syncMgr, uiState);
    markAllClean();
    delay(ms);
}

void test_ui_demo_sequence() {
    TEST_ASSERT_TRUE(display.begin());
    display.clear();

    // BOOT sequence
    ecu.reset();
    ecu.lastUpdateMillis = 0; 
    syncMgr.update(ecu); 
    renderTick(1000);

    // WAIT ECU
    ecu.lastUpdateMillis = millis(); 
    ecu.isDataValid = false; 
    ecu.isSynced = false; 
    syncMgr.update(ecu); 
    renderTick(1200);

    // SYNCING
    ecu.isDataValid = true; 
    ecu.isSynced = false; 
    ecu.lastUpdateMillis = millis(); 
    syncMgr.update(ecu); 
    renderTick(1200);

    // === ANIMATED DEMO: semua nilai bergerak dinamis ===
    ecu.isSynced = true;
    ecu.isDataValid = true;
    ecu.battery = 13900;
    
    // Siklus 1: Idle → cruising (NORMAL)
    for (int i = 0; i < 30; i++) {
        ecu.rpm = 900 + i * 50;           // 900 → 2400 rpm
        ecu.map = 30 + i * 1;             // 30 → 60 kPa
        ecu.clt = 70 + i * 0.5;           // 70 → 85°C
        ecu.iat = 25 + i * 0.3;           // 25 → 34°C
        ecu.afr = 1450 - i * 2;           // 14.5 → 13.9
        ecu.tps = 2 + i * 1;              // 2 → 32%
        ecu.lastUpdateMillis = millis();
        syncMgr.update(ecu);
        renderTick(100);
    }
    
    // Siklus 2: Heavy load → overheat (CAUTION/WARNING)
    for (int i = 0; i < 40; i++) {
        ecu.rpm = 2400 + i * 100;         // 2400 → 6400 rpm
        ecu.map = 60 + i * 2;             // 60 → 140 kPa
        ecu.clt = 85 + i * 0.7;           // 85 → 113°C (WARNING!)
        ecu.iat = 34 + i * 0.5;           // 34 → 54°C
        ecu.afr = 1390 - i * 5;           // 13.9 → 11.9 (RICH!)
        ecu.tps = 32 + i * 1.5;           // 32 → 92%
        ecu.battery = 13900 - i * 30;     // voltage drop
        ecu.lastUpdateMillis = millis();
        syncMgr.update(ecu);
        renderTick(100);
    }
    
    // Siklus 3: Decel → idle (recovery)
    for (int i = 0; i < 35; i++) {
        ecu.rpm = 6400 - i * 150;         // 6400 → 1150 rpm
        ecu.map = 140 - i * 3;            // 140 → 35 kPa
        ecu.clt = 113 - i * 0.8;          // 113 → 85°C (cooling)
        ecu.iat = 54 - i * 0.6;           // 54 → 33°C
        ecu.afr = 1190 + i * 8;           // 11.9 → 14.7 (LEAN!)
        ecu.tps = 92 - i * 2.5;           // 92 → 4%
        ecu.battery = 12700 + i * 20;     // voltage recover
        ecu.lastUpdateMillis = millis();
        syncMgr.update(ecu);
        renderTick(80);
    }
    
    // Siklus 4: Fluctuation at idle (NORMAL)
    for (int i = 0; i < 25; i++) {
        ecu.rpm = 900 + (i % 5) * 20;     // 900±100 rpm oscillation
        ecu.map = 35 + (i % 3) * 2;       // 35±6 kPa
        ecu.clt = 85 + (i % 4);           // stable CLT
        ecu.iat = 30 + (i % 2);           // stable IAT
        ecu.afr = 1450 + (i % 5) * 5;     // 14.5±0.25 afr
        ecu.tps = 2 + (i % 2);            // idle TPS
        ecu.lastUpdateMillis = millis();
        syncMgr.update(ecu);
        renderTick(150);
    }

    // SYNC LOSS scenario
    syncMgr.triggerSyncLoss(); 
    for (int i = 0; i < 10; ++i) { 
        renderTick(250); 
    }

    // Recovery
    ecu.isDataValid = true; 
    ecu.isSynced = true; 
    ecu.lastUpdateMillis = millis(); 
    syncMgr.update(ecu);
    for (int i = 0; i < 15; ++i) { 
        renderTick(150); 
    }

    TEST_PASS();
}

void setup() { UNITY_BEGIN(); RUN_TEST(test_ui_demo_sequence); }
void loop() { UNITY_END(); }
