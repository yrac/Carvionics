#ifndef UI_STATE_MACHINE_H
#define UI_STATE_MACHINE_H

#include <Arduino.h>
#include <stdint.h>
#include "SyncManager.h"

/**
 * @class UIStateMachine
 * @brief Mengelola UI state dan rendering logic
 * 
 * State machine untuk koordinasi rendering:
 * - Mendeteksi state change (dari SyncManager)
 * - Set dirty-flag untuk slice yang perlu redraw
 * - Manage blink state untuk SYNC_LOSS mode
 * 
 * Slice categories:
 * - HEADER: Status mesin & sync info
 * - RPM_FIELD: Primary RPM display
 * - ENGINE_CORE: CLT, AFR, MAP, Battery
 * - CONTROL_DATA: TPS, IAT
 * - FOOTER: System message
 * - FULL_SCREEN: SYNC_LOSS override
 */
class UIStateMachine {
public:
    enum class UISlice {
        HEADER,
        RPM_FIELD,
        ENGINE_CORE,
        CONTROL_DATA,
        FOOTER,
        FULL_SCREEN  // SYNC_LOSS override
    };
    
    enum class BlinkState {
        OFF,
        ON,
        BLINKING
    };
    
    UIStateMachine();
    
    // Update state machine dengan sync state baru
    void update(SyncManager::SyncState sync_state);
    
    // Check jika slice perlu redraw
    bool isSliceDirty(UISlice slice) const;
    
    // Clear dirty flag untuk slice (setelah redraw)
    void markSliceClean(UISlice slice);
    
    // Get blink state untuk SYNC_LOSS visual effect
    BlinkState getBlinkState() const { return blink_state_; }
    bool shouldBlinkOn() const;
    
    // Get current display mode
    SyncManager::SyncState getCurrentUIState() const { return current_ui_state_; }
    
    // Blink period configuration (ms)
    void setBlinkPeriod(uint16_t period_ms) { blink_period_ms_ = period_ms; }
    uint16_t getBlinkPeriod() const { return blink_period_ms_; }

private:
    SyncManager::SyncState current_ui_state_;
    SyncManager::SyncState previous_ui_state_;
    
    // Dirty flags untuk setiap slice
    uint8_t dirty_flags_;
    
    // Blinking logic untuk SYNC_LOSS
    BlinkState blink_state_;
    uint32_t blink_timestamp_;
    uint16_t blink_period_ms_;
    bool last_blink_on_;
    
    // Slice constants
    static constexpr uint8_t SLICE_HEADER = 0;
    static constexpr uint8_t SLICE_RPM = 1;
    static constexpr uint8_t SLICE_ENGINE = 2;
    static constexpr uint8_t SLICE_CONTROL = 3;
    static constexpr uint8_t SLICE_FOOTER = 4;
    static constexpr uint8_t SLICE_FULLSCREEN = 5;
    
    void markAllDirty_();
    void clearAllDirty_();
    void updateBlinkState_();
};

#endif
