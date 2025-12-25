#ifndef SYNC_MANAGER_H
#define SYNC_MANAGER_H

#include <Arduino.h>
#include <stdint.h>
#include "ECUData.h"

/**
 * @class SyncManager
 * @brief Mengelola sync loss detection & recovery logic
 * 
 * State machine untuk mendeteksi kehilangan data (sync loss) dan
 * recovery delay sebelum kembali ke normal.
 * 
 * Deteksi sync loss:
 * - Data timeout (no update > 500ms)
 * - Sync counter increment (ECU report)
 * - Threshold violation (CLT, AFR, battery)
 */
class SyncManager {
public:
    enum class SyncState {
        NO_DATA,        // Data timeout / tidak ada data terkini
        NORMAL,
        CAUTION,        // Warning threshold exceeded
        WARNING,        // Critical threshold exceeded
        SYNC_LOSS,      // Full data loss or sync counter increment
        RECOVERY        // Waiting for stability before returning to NORMAL
    };
    
    // Threshold configuration (tunable)
    struct Thresholds {
        uint16_t rpm_max;           // Max safe RPM [8000]
        int16_t clt_min, clt_max;   // Coolant temp range [10, 110]
        uint16_t afr_min, afr_max;  // AFR range [1200, 1700] = 12.00:1 to 17.00:1
        uint16_t battery_min;       // Min battery voltage [11000 mV = 11V]
        uint32_t data_timeout_ms;   // No data timeout [500ms]
        uint32_t recovery_delay_ms; // Recovery stabilization time [2000ms]
    };
    
    SyncManager();
    
    // Set custom thresholds
    void setThresholds(const Thresholds &thresholds);
    // Get thresholds (read-only)
    const Thresholds& getThresholds() const { return thresholds_; }
    
    // Get current state
    SyncState getState() const { return current_state_; }
    const char* getStateString() const;
    
    // Update sync state based on current ECU data
    void update(const ECUData &ecu_data);
    
    // Manual sync loss trigger (e.g., dari communication error)
    void triggerSyncLoss();
    
    // Get recovery progress (0..100%)
    uint8_t getRecoveryProgress() const;
    
    // Check if UI should force redraw (state change)
    bool isStateChanged() const { return state_changed_; }
    
    // Get time in current state (ms)
    uint32_t getStateElapsedTime() const;
    
    // Debug output
    void debugPrint() const;

private:
    SyncState current_state_;
    SyncState previous_state_;
    bool state_changed_;
    
    uint32_t state_enter_time_;    // Timestamp saat masuk state
    uint16_t previous_sync_counter_;
    
    Thresholds thresholds_;
    
    // Private state machine logic
    void evaluateThresholds_(const ECUData &ecu_data);
    void handleStateTransition_(SyncState new_state);
    bool hasValidData_(const ECUData &ecu_data) const;
};

#endif
