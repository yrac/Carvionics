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
    // Init display
    TEST_ASSERT_TRUE(display.begin());
    display.clear();

    // 1) BOOT / POWER ON
    ecu.reset();
    ecu.lastUpdateMillis = 0;
    syncMgr.update(ecu);
    renderTick(800);

    // 2) WAIT ECU (NO DATA)
    ecu.lastUpdateMillis = millis();
    ecu.isDataValid = false;
    ecu.isSynced = false;
    syncMgr.update(ecu);
    renderTick(1200);

    // 3) SYNCING (DATA MASUK, BELUM STABIL)
    ecu.isDataValid = true;
    ecu.isSynced = false;
    ecu.lastUpdateMillis = millis();
    syncMgr.update(ecu);
    renderTick(1200);

    // 4) NORMAL OPERATION
    ecu.rpm = 2450; ecu.map = 42; ecu.clt = 87; ecu.iat = 31; ecu.afr = 1440; ecu.tps = 3; ecu.battery = 13900;
    ecu.isSynced = true; ecu.isDataValid = true; ecu.lastUpdateMillis = millis();
    syncMgr.update(ecu);
    renderTick(1600);

    // 5) CAUTION STATE (CLT tinggi / AFR lean)
    ecu.clt = 97; ecu.afr = 1620; ecu.battery = 13400;
    syncMgr.update(ecu);
    renderTick(1600);

    // 6) WARNING STATE (Overheat / battery drop)
    ecu.clt = 108; ecu.afr = 1590; ecu.battery = 11800;
    ecu.rpm = 2600; ecu.map = 55; ecu.tps = 12; ecu.iat = 38;
    syncMgr.update(ecu);
    renderTick(1600);

    // 7) SYNC LOSS (FULL OVERRIDE)
    syncMgr.triggerSyncLoss();
    for (int i = 0; i < 8; ++i) {
        renderTick(250); // let blink toggle drive FULL_SCREEN redraws
    }

    // 8) RECOVERY → NORMAL
    ecu.isDataValid = true; ecu.isSynced = true; ecu.lastUpdateMillis = millis();
    syncMgr.update(ecu); // will go RECOVERY then NORMAL after delay
    for (int i = 0; i < 10; ++i) {
        renderTick(200);
    }

    TEST_PASS();
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_ui_demo_sequence);
}

void loop() {
    UNITY_END();
}
