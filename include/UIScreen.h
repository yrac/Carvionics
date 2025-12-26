#ifndef UI_SCREEN_H
#define UI_SCREEN_H

#include <Arduino.h>
#include <stdint.h>
#include "DisplayManager.h"
#include "ECUData.h"
#include "SyncManager.h"
#include "UIStateMachine.h"

/**
 * @class UIScreen
 * @brief Renderer untuk UI slices (aircraft cockpit EFIS style)
 * 
 * Mengelola drawing untuk setiap UI slice:
 * - Header: Status, sync indicator, engine status
 * - RPM Field: Primary display (dominan)
 * - Engine Core: CLT, AFR, MAP, Battery (4-quadrant)
 * - Control Data: TPS, IAT, diagnostics
 * - Footer: System message, frame rate, data status
 * 
 * Layout (320x240):
 * ┌─────────────────────────────────────────┐
 * │ HEADER (y=0..30, 30px)                  │
 * ├─────────────────────────────────────────┤
 * │ RPM FIELD (y=30..120, 90px)             │
 * ├──────────────┬──────────────────────────┤
 * │ CLT / AFR    │ MAP / BATT               │
 * │ (y=120..180) │ (y=120..180)             │
 * ├──────────────┼──────────────────────────┤
 * │ TPS / IAT    │ SYNC / HEALTH            │
 * │ (y=180..220) │ (y=180..220)             │
 * ├─────────────────────────────────────────┤
 * │ FOOTER (y=220..240, 20px)               │
 * └─────────────────────────────────────────┘
 */
class UIScreen {
public:
    UIScreen(DisplayManager &display);
    
    // Full screen update
    void render(const ECUData &ecu_data,
                const SyncManager &sync_mgr,
                const UIStateMachine &ui_state);
    
    // Individual slice rendering
    void renderHeader_(const ECUData &ecu_data, 
                      const SyncManager &sync_mgr,
                      const UIStateMachine &ui_state);
    
    void renderRPMField_(const ECUData &ecu_data,
                       const SyncManager &sync_mgr);
    
    void renderEngineCore_(const ECUData &ecu_data,
                          const SyncManager &sync_mgr);
    
    void renderControlData_(const ECUData &ecu_data,
                           const SyncManager &sync_mgr);
    
    void renderFooter_(const ECUData &ecu_data,
                      const SyncManager &sync_mgr,
                      const UIStateMachine &ui_state);
    
    void renderSyncLossScreen_(const SyncManager &sync_mgr,
                              const UIStateMachine &ui_state);

private:
    DisplayManager &display_;

    // Smoothed display values to make transitions more seamless
    struct Smooth {
        bool initialized = false;
        float rpm = 0;
        float map = 0;
        float clt = 0;
        float iat = 0;
        float afr_x100 = 0; // store AFR scaled by 100
        float tps = 0;
        float battery = 0; // in mV
    } smooth_;

    // Previous value strings per cell to skip unnecessary redraws
    // Order: [RPM, MAP, CLT, IAT, AFR, TPS]
    char prevVals_[6][16] = {{0}};

    void updateSmooth_(const ECUData &ecu);
    static float lerp_(float from, float to, float alpha);
    
    // Layout constants (pixel coordinates) — 3x2 grid sama rata (six-pack avionics)
    static constexpr uint16_t HEADER_Y = 0;
    static constexpr uint16_t HEADER_H = 20;   // tipis
    
    // Grid 3 kolom x 2 baris (RPM/MAP/CLT di atas, IAT/AFR/TPS di bawah)
    static constexpr uint16_t GRID_ROW1_Y = HEADER_Y + HEADER_H;
    static constexpr uint16_t GRID_ROW1_H = 100;  // Row-1 height
    
    static constexpr uint16_t GRID_ROW2_Y = GRID_ROW1_Y + GRID_ROW1_H;
    static constexpr uint16_t GRID_ROW2_H = 100;  // Row-2 height (same as row-1)
    
    static constexpr uint16_t FOOTER_Y = GRID_ROW2_Y + GRID_ROW2_H;
    static constexpr uint16_t FOOTER_H = 240 - FOOTER_Y; // auto-calc; ~20px
    
    // 3 kolom sama lebar
    static constexpr uint16_t CELL_W = 320 / 3;  // ~106px per cell
    
    // Helper methods
    DisplayManager::Color getStateColor_(SyncManager::SyncState state) const;
    const char* getStateName_(SyncManager::SyncState state) const;
    void drawBox_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                  DisplayManager::Color border, DisplayManager::Color fill);
    void drawParameterBox_(uint16_t x, uint16_t y, const char *label,
                          const char *value, DisplayManager::Color fg);

    // Helper grid rendering
    void drawGridCell_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                       const char* label, const char* value,
                       DisplayManager::Color labelColor,
                       DisplayManager::Color valueColor,
                       DisplayManager::Color borderColor = DisplayManager::Color::LIGHT_GRAY,
                       uint8_t valueSize = 2);
    void drawGridValueArea_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                            const char* value,
                            DisplayManager::Color valueColor,
                            uint8_t valueSize);
    const char* headerEcuStatusText_(const ECUData &ecu_data, const SyncManager &sync_mgr) const;

    // State-driven color helpers
    DisplayManager::Color valueColorForState_(SyncManager::SyncState state) const;
    DisplayManager::Color borderColorForState_(SyncManager::SyncState state) const;

    // Fullscreen state renderers
    void renderBootSelfTest_();
    void renderWaitECU_();
    void renderSyncing_();
    void renderRecovery_();

    // Annotation helpers
    void annotateClt_(char *buf, size_t bufSize, int16_t clt, const SyncManager &sync_mgr) const;
    void annotateAfr_(char *buf, size_t bufSize, uint16_t afr, const SyncManager &sync_mgr) const;
};

#endif
