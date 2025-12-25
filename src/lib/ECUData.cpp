#include "ECUData.h"

ECUData::ECUData() {
    reset();
}

void ECUData::reset() {
    rpm = 0;
    clt = 0;
    afr = 1400;  // 14.00:1
    map = 0;
    tps = 0;
    iat = 0;
    battery = 12000;  // 12V
    
    syncLossCounter = 0;
    isSynced = false;
    lastUpdateMillis = 0;
    isDataValid = false;
}

bool ECUData::isStale(uint32_t timeout_ms) const {
    if (!isDataValid) return true;
    return (millis() - lastUpdateMillis) > timeout_ms;
}

void ECUData::debugPrint() const {
    Serial.println("\n=== ECU Data ===");
    Serial.print("RPM: "); Serial.println(rpm);
    Serial.print("CLT: "); Serial.print(clt); Serial.println(" C");
    Serial.print("AFR: "); Serial.print(afr/100.0); Serial.println(":1");
    Serial.print("MAP: "); Serial.print(map); Serial.println(" kPa");
    Serial.print("TPS: "); Serial.print(tps); Serial.println(" %");
    Serial.print("IAT: "); Serial.print(iat); Serial.println(" C");
    Serial.print("Battery: "); Serial.print(battery/1000.0); Serial.println(" V");
    Serial.print("SyncLoss Counter: "); Serial.println(syncLossCounter);
    Serial.print("Synced: "); Serial.println(isSynced ? "Yes" : "No");
    Serial.print("Data Valid: "); Serial.println(isDataValid ? "Yes" : "No");
    Serial.print("Stale: "); Serial.println(isStale() ? "Yes" : "No");
}
