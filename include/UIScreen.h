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
    
    // Layout constants (pixel coordinates)
    static constexpr uint16_t HEADER_Y = 0;
    static constexpr uint16_t HEADER_H = 24;   // lebih tipis, avionics bar
    
    // Tiga baris grid avionics, masing-masing 60px
    static constexpr uint16_t RPM_FIELD_Y = HEADER_Y + HEADER_H; // Row-1 (RPM/MAP)
    static constexpr uint16_t RPM_FIELD_H = 60;
    
    static constexpr uint16_t ENGINE_CORE_Y = RPM_FIELD_Y + RPM_FIELD_H; // Row-2 (CLT/IAT)
    static constexpr uint16_t ENGINE_CORE_H = 60;
    
    static constexpr uint16_t CONTROL_DATA_Y = ENGINE_CORE_Y + ENGINE_CORE_H; // Row-3 (AFR/TPS + secondary)
    static constexpr uint16_t CONTROL_DATA_H = 60;
    
    static constexpr uint16_t FOOTER_Y = CONTROL_DATA_Y + CONTROL_DATA_H;
    static constexpr uint16_t FOOTER_H = 240 - FOOTER_Y; // sisakan sisa tinggi layar
    
    static constexpr uint16_t LEFT_PANEL_W = 160;  // dua kolom
    static constexpr uint16_t RIGHT_PANEL_W = 160;
    
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
