#include "SyncManager.h"

SyncManager::SyncManager()
    : current_state_(SyncState::NO_DATA),
      previous_state_(SyncState::NORMAL),
      state_changed_(false),
      state_enter_time_(0),
      previous_sync_counter_(0) {
    
    // Default thresholds (tuned untuk automotive use)
    thresholds_.rpm_max = 8000;
    thresholds_.clt_min = 10;
    thresholds_.clt_max = 110;
    thresholds_.afr_min = 1200;     // 12.00:1
    thresholds_.afr_max = 1700;     // 17.00:1
    thresholds_.battery_min = 11000; // 11V
    thresholds_.data_timeout_ms = 500;
    thresholds_.recovery_delay_ms = 2000;
}

void SyncManager::setThresholds(const Thresholds &thresholds) {
    thresholds_ = thresholds;
}

const char* SyncManager::getStateString() const {
    switch (current_state_) {
        case SyncState::NO_DATA:  return "NO_DATA";
        case SyncState::NORMAL:    return "NORMAL";
        case SyncState::CAUTION:   return "CAUTION";
        case SyncState::WARNING:   return "WARNING";
        case SyncState::SYNC_LOSS: return "SYNC_LOSS";
        case SyncState::RECOVERY:  return "RECOVERY";
        default:                   return "UNKNOWN";
    }
}

void SyncManager::update(const ECUData &ecu_data) {
    state_changed_ = false;
    
    // Check data timeout (NO_DATA state)
    if (ecu_data.isStale(thresholds_.data_timeout_ms)) {
        handleStateTransition_(SyncState::NO_DATA);
        return;
    }
    
    // Check sync counter change
    if (ecu_data.syncLossCounter > previous_sync_counter_) {
        previous_sync_counter_ = ecu_data.syncLossCounter;
        handleStateTransition_(SyncState::SYNC_LOSS);
        return;
    }
    
    // Evaluate thresholds
    evaluateThresholds_(ecu_data);
    
    // Recovery logic: setelah SYNC_LOSS atau NO_DATA, tunggu stabil sebelum NORMAL
    if (current_state_ == SyncState::RECOVERY) {
        uint32_t recovery_elapsed = millis() - state_enter_time_;
        if (recovery_elapsed >= thresholds_.recovery_delay_ms) {
            handleStateTransition_(SyncState::NORMAL);
        }
    }
    else if (current_state_ == SyncState::SYNC_LOSS || current_state_ == SyncState::NO_DATA) {
        // Jika data kembali valid, masuk RECOVERY terlebih dahulu
        if (hasValidData_(ecu_data)) {
            handleStateTransition_(SyncState::RECOVERY);
        }
    }
}

void SyncManager::evaluateThresholds_(const ECUData &ecu_data) {
    int warning_count = 0;
    int caution_count = 0;
    
    // CLT check
    if (ecu_data.clt < thresholds_.clt_min || ecu_data.clt > thresholds_.clt_max) {
        warning_count++;
    } else if (ecu_data.clt < thresholds_.clt_min + 5 || ecu_data.clt > thresholds_.clt_max - 5) {
        caution_count++;
    }
    
    // AFR check
    if (ecu_data.afr < thresholds_.afr_min || ecu_data.afr > thresholds_.afr_max) {
        warning_count++;
    } else if (ecu_data.afr < thresholds_.afr_min + 100 || ecu_data.afr > thresholds_.afr_max - 100) {
        caution_count++;
    }
    
    // Battery check
    if (ecu_data.battery < thresholds_.battery_min) {
        warning_count++;
    } else if (ecu_data.battery < thresholds_.battery_min + 500) {  // 11.5V
        caution_count++;
    }
    
    // State transition based on warnings
    SyncState new_state = SyncState::NORMAL;
    
    if (warning_count >= 2) {
        new_state = SyncState::WARNING;
    } else if (warning_count == 1 || caution_count >= 2) {
        new_state = SyncState::WARNING;
    } else if (caution_count >= 1) {
        new_state = SyncState::CAUTION;
    }
    
    if (new_state != current_state_) {
        handleStateTransition_(new_state);
    }
}

void SyncManager::triggerSyncLoss() {
    handleStateTransition_(SyncState::SYNC_LOSS);
}

uint8_t SyncManager::getRecoveryProgress() const {
    if (current_state_ != SyncState::RECOVERY) return 0;
    
    uint32_t elapsed = millis() - state_enter_time_;
    uint32_t progress = (elapsed * 100) / thresholds_.recovery_delay_ms;
    return (progress > 100) ? 100 : progress;
}

uint32_t SyncManager::getStateElapsedTime() const {
    return millis() - state_enter_time_;
}

void SyncManager::handleStateTransition_(SyncState new_state) {
    if (new_state == current_state_) return;
    
    previous_state_ = current_state_;
    current_state_ = new_state;
    state_enter_time_ = millis();
    state_changed_ = true;
    
    // Debug output
    Serial.print("[SyncManager] State: ");
    Serial.print(getStateString());
    Serial.print(" (from ");
    switch (previous_state_) {
        case SyncState::NORMAL:    Serial.print("NORMAL"); break;
        case SyncState::CAUTION:   Serial.print("CAUTION"); break;
        case SyncState::WARNING:   Serial.print("WARNING"); break;
        case SyncState::SYNC_LOSS: Serial.print("SYNC_LOSS"); break;
        case SyncState::RECOVERY:  Serial.print("RECOVERY"); break;
        default: Serial.print("UNKNOWN");
    }
    Serial.println(")");
}

void SyncManager::debugPrint() const {
    Serial.println("\n=== SyncManager ===");
    Serial.print("State: "); Serial.println(getStateString());
    Serial.print("Elapsed: "); Serial.print(getStateElapsedTime()); Serial.println(" ms");
    if (current_state_ == SyncState::RECOVERY) {
        Serial.print("Recovery Progress: "); Serial.print(getRecoveryProgress()); Serial.println("%");
    }
}

bool SyncManager::hasValidData_(const ECUData &ecu_data) const {
    // Data dianggap valid jika data tidak stale dan flag valid diset
    return ecu_data.isDataValid && !ecu_data.isStale(thresholds_.data_timeout_ms);
}
