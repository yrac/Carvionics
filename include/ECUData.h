#ifndef ECU_DATA_H
#define ECU_DATA_H

#include <Arduino.h>
#include <stdint.h>

/**
 * @class ECUData
 * @brief Struktur data mesin dari Speeduino (read-only)
 * 
 * Menyimpan parameter motor utama yang diparsing dari frame Speeduino.
 * Data ini diupdate secara real-time dari serial parser.
 */
class ECUData {
public:
    // Engine State Parameters
    uint16_t rpm;           // RPM [0..8000+]
    int16_t clt;            // Coolant Temperature [°C, -40..120]
    uint16_t afr;           // Air-Fuel Ratio [100x, e.g., 1400 = 14.00:1]
    uint16_t map;           // Manifold Absolute Pressure [kPa, 0..255]
    uint16_t tps;           // Throttle Position Sensor [0..100%]
    int16_t iat;            // Intake Air Temperature [°C, -40..120]
    uint16_t battery;       // Battery Voltage [mV, 0..16000]
    
    // Sync Status
    uint16_t syncLossCounter;  // Total sync loss events
    bool isSynced;             // Current sync status
    
    // Timestamp & Health
    uint32_t lastUpdateMillis; // Last successful frame received
    bool isDataValid;          // Data still within timeout window
    
    // Constructor
    ECUData();
    
    // Reset data to default/safe state
    void reset();
    
    // Check if data is stale (no updates for timeout_ms)
    bool isStale(uint32_t timeout_ms = 500) const;
    
    // Print debug info to Serial
    void debugPrint() const;
};

#endif
