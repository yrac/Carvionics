#include "UIStateMachine.h"

UIStateMachine::UIStateMachine()
    : current_ui_state_(SyncManager::SyncState::NORMAL),
      previous_ui_state_(SyncManager::SyncState::NORMAL),
      dirty_flags_(0xFF),  // All slices dirty on startup
      blink_state_(BlinkState::OFF),
      blink_timestamp_(0),
      blink_period_ms_(500) {
}

void UIStateMachine::update(SyncManager::SyncState sync_state) {
    previous_ui_state_ = current_ui_state_;
    current_ui_state_ = sync_state;
    
    // State change detection
    if (sync_state != previous_ui_state_) {
        markAllDirty_();
        
        // Initialize blink state untuk SYNC_LOSS
        if (sync_state == SyncManager::SyncState::SYNC_LOSS) {
            blink_state_ = BlinkState::BLINKING;
            blink_timestamp_ = millis();
        } else {
            blink_state_ = BlinkState::OFF;
        }
    }
    
    // Update blinking
    updateBlinkState_();
}

bool UIStateMachine::isSliceDirty(UISlice slice) const {
    uint8_t bit = (uint8_t)slice;
    return (dirty_flags_ & (1 << bit)) != 0;
}

void UIStateMachine::markSliceClean(UISlice slice) {
    uint8_t bit = (uint8_t)slice;
    dirty_flags_ &= ~(1 << bit);
}

bool UIStateMachine::shouldBlinkOn() const {
    if (blink_state_ != BlinkState::BLINKING) {
        return blink_state_ == BlinkState::ON;
    }
    
    // Calculate blink cycle
    uint32_t elapsed = millis() - blink_timestamp_;
    uint32_t cycle_pos = elapsed % (blink_period_ms_ * 2);
    return cycle_pos < blink_period_ms_;
}

void UIStateMachine::markAllDirty_() {
    dirty_flags_ = 0xFF;
}

void UIStateMachine::clearAllDirty_() {
    dirty_flags_ = 0x00;
}

void UIStateMachine::updateBlinkState_() {
    if (blink_state_ == BlinkState::BLINKING) {
        // Blinking terus selama SYNC_LOSS
        if (current_ui_state_ != SyncManager::SyncState::SYNC_LOSS) {
            blink_state_ = BlinkState::OFF;
        }
    }
}
