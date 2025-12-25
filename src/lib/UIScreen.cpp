#include "UIScreen.h"

UIScreen::UIScreen(DisplayManager &display)
    : display_(display) {
}

void UIScreen::render(const ECUData &ecu_data,
                      const SyncManager &sync_mgr,
                      const UIStateMachine &ui_state) {
    
    // SYNC_LOSS override: full-screen warning
    if (sync_mgr.getState() == SyncManager::SyncState::SYNC_LOSS) {
        renderSyncLossScreen_(sync_mgr, ui_state);
        return;
    }

    // BOOT: perangkat baru menyala, belum ada data
    if (ecu_data.lastUpdateMillis == 0) {
        renderBootSelfTest_();
        return;
    }

    // WAIT ECU / NO DATA: belum ada frame valid
    if (!ecu_data.isDataValid) {
        renderWaitECU_();
        return;
    }

    // SYNCING: data masuk tapi belum sinkron
    if (!ecu_data.isSynced) {
        renderSyncing_();
        return;
    }

    // RECOVERY: masa pemulihan, tampilkan layar khusus
    if (sync_mgr.getState() == SyncManager::SyncState::RECOVERY) {
        renderRecovery_();
        return;
    }
    
    // Normal rendering dengan dirty-flag optimization
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::HEADER)) {
        renderHeader_(ecu_data, sync_mgr, ui_state);
    }
    
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::RPM_FIELD)) {
        renderRPMField_(ecu_data, sync_mgr);
    }
    
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::ENGINE_CORE)) {
        renderEngineCore_(ecu_data, sync_mgr);
    }
    
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::CONTROL_DATA)) {
        renderControlData_(ecu_data, sync_mgr);
    }
    
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::FOOTER)) {
        renderFooter_(ecu_data, sync_mgr, ui_state);
    }
}

void UIScreen::renderHeader_(const ECUData &ecu_data,
                             const SyncManager &sync_mgr,
                             const UIStateMachine &ui_state) {
    // Header bar: ECU | SYNC | BAT
    display_.fillRect(0, HEADER_Y, 320, HEADER_H, DisplayManager::Color::DARK_GRAY);
    display_.drawRect(0, HEADER_Y, 320, HEADER_H, DisplayManager::Color::WHITE);

    // Tiga kolom seimbang
    const uint16_t colW = 106; // ~320/3
    const uint16_t padX = 6;
    const uint16_t baseY = HEADER_Y + 6;

    // ECU status (kiri)
    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::DARK_GRAY);
    display_.setCursor(padX, baseY);
    display_.print("ECU");
    display_.setCursor(padX + 34, baseY);
    const char* ecuTxt = headerEcuStatusText_(ecu_data, sync_mgr);
    DisplayManager::Color ecuCol = ecu_data.isDataValid ? DisplayManager::Color::GREEN : DisplayManager::Color::AMBER;
    display_.setTextColor(ecuCol, DisplayManager::Color::DARK_GRAY);
    display_.print(ecuTxt);

    // Jika RECOVERY, tampilkan progress kecil di sebelahnya
    if (sync_mgr.getState() == SyncManager::SyncState::RECOVERY) {
        display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::DARK_GRAY);
        display_.setCursor(padX + 70, baseY);
        char recStr[12];
        snprintf(recStr, sizeof(recStr), "%d%%", sync_mgr.getRecoveryProgress());
        display_.print(recStr);
    }

    // SYNC (tengah)
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::DARK_GRAY);
    display_.setCursor(colW + padX, baseY);
    display_.print("SYNC");
    display_.setCursor(colW + padX + 40, baseY);
    char syncStr[16];
    snprintf(syncStr, sizeof(syncStr), "%d", ecu_data.syncLossCounter);
    display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::DARK_GRAY);
    display_.print(syncStr);

    // BAT (kanan)
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::DARK_GRAY);
    display_.setCursor(colW*2 + padX, baseY);
    display_.print("BAT");
    display_.setCursor(colW*2 + padX + 30, baseY);
    char batStr[16];
    snprintf(batStr, sizeof(batStr), "%.1fV", ecu_data.battery / 1000.0);
    // Warna hanya untuk status: nilai tetap putih
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::DARK_GRAY);
    display_.print(batStr);
}

void UIScreen::renderRPMField_(const ECUData &ecu_data,
                              const SyncManager &sync_mgr) {
    // Row-1: RPM | MAP (dua kolom)
    display_.fillRect(0, RPM_FIELD_Y, 320, RPM_FIELD_H, DisplayManager::Color::BLACK);

    // RPM cell (kiri)
    char rpmStr[16]; snprintf(rpmStr, sizeof(rpmStr), "%d", ecu_data.rpm);
    DisplayManager::Color valCol1 = DisplayManager::Color::WHITE; // nilai selalu putih
    DisplayManager::Color brdCol1 = borderColorForState_(sync_mgr.getState());
    if (sync_mgr.getState() == SyncManager::SyncState::NO_DATA || !ecu_data.isDataValid) {
        strcpy(rpmStr, "--");
    }
    drawGridCell_(0, RPM_FIELD_Y, LEFT_PANEL_W, RPM_FIELD_H,
                  "RPM", rpmStr,
                  DisplayManager::Color::WHITE,
                  valCol1,
                  brdCol1,
                  2);

    // MAP cell (kanan)
    char mapStr[16]; snprintf(mapStr, sizeof(mapStr), "%d", ecu_data.map);
    DisplayManager::Color valCol2 = DisplayManager::Color::WHITE;
    DisplayManager::Color brdCol2 = borderColorForState_(sync_mgr.getState());
    if (sync_mgr.getState() == SyncManager::SyncState::NO_DATA || !ecu_data.isDataValid) {
        strcpy(mapStr, "--");
    }
    drawGridCell_(LEFT_PANEL_W, RPM_FIELD_Y, RIGHT_PANEL_W, RPM_FIELD_H,
                  "MAP", mapStr,
                  DisplayManager::Color::WHITE,
                  valCol2,
                  brdCol2,
                  2);
}

void UIScreen::renderEngineCore_(const ECUData &ecu_data,
                                const SyncManager &sync_mgr) {
    // Row-2: CLT | IAT
    display_.fillRect(0, ENGINE_CORE_Y, 320, ENGINE_CORE_H, DisplayManager::Color::BLACK);

    char cltStr[16];
    if (!ecu_data.isDataValid) {
        strcpy(cltStr, "--");
    } else {
        annotateClt_(cltStr, sizeof(cltStr), ecu_data.clt, sync_mgr);
    }
    drawGridCell_(0, ENGINE_CORE_Y, LEFT_PANEL_W, ENGINE_CORE_H,
                  "CLT", cltStr,
                  DisplayManager::Color::WHITE,
                  DisplayManager::Color::WHITE,
                  borderColorForState_(sync_mgr.getState()),
                  2);

    char iatStr[16]; snprintf(iatStr, sizeof(iatStr), "%d", ecu_data.iat);
    if (!ecu_data.isDataValid) {
        strcpy(iatStr, "--");
    }
    drawGridCell_(LEFT_PANEL_W, ENGINE_CORE_Y, RIGHT_PANEL_W, ENGINE_CORE_H,
                  "IAT", iatStr,
                  DisplayManager::Color::WHITE,
                  DisplayManager::Color::WHITE,
                  borderColorForState_(sync_mgr.getState()),
                  2);
}

void UIScreen::renderControlData_(const ECUData &ecu_data,
                                 const SyncManager &sync_mgr) {
    // Row-3: AFR | TPS + Secondary (ADV/DWL/ISC kecil di bawah)
    display_.fillRect(0, CONTROL_DATA_Y, 320, CONTROL_DATA_H, DisplayManager::Color::BLACK);

    // AFR
    char afrStr[16];
    if (!ecu_data.isDataValid) {
        strcpy(afrStr, "--");
    } else {
        annotateAfr_(afrStr, sizeof(afrStr), ecu_data.afr, sync_mgr);
    }
    drawGridCell_(0, CONTROL_DATA_Y, LEFT_PANEL_W, CONTROL_DATA_H - 18,
                  "AFR", afrStr,
                  DisplayManager::Color::WHITE,
                  DisplayManager::Color::WHITE,
                  borderColorForState_(sync_mgr.getState()),
                  2);

    // TPS
    char tpsStr[16]; snprintf(tpsStr, sizeof(tpsStr), "%d", ecu_data.tps);
    if (!ecu_data.isDataValid) {
        strcpy(tpsStr, "--");
    }
    drawGridCell_(LEFT_PANEL_W, CONTROL_DATA_Y, RIGHT_PANEL_W, CONTROL_DATA_H - 18,
                  "TPS", tpsStr,
                  DisplayManager::Color::WHITE,
                  DisplayManager::Color::WHITE,
                  borderColorForState_(sync_mgr.getState()),
                  2);

    // Secondary line (kecil) di dasar slice
    const uint16_t secY = CONTROL_DATA_Y + CONTROL_DATA_H - 16;
    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::CYAN, DisplayManager::Color::BLACK);
    display_.setCursor(6, secY);
    // Placeholder ADV/DWL/ISC (anggap tersedia; jika belum, tampilkan --)
    display_.print("ADV:-- DWL:--");
    display_.setCursor(LEFT_PANEL_W + 6, secY);
    display_.print("ISC:--");
}

void UIScreen::renderFooter_(const ECUData &ecu_data,
                            const SyncManager &sync_mgr,
                            const UIStateMachine &ui_state) {
    // Footer: Status atau FUEL x.xx L (placeholder)
    display_.fillRect(0, FOOTER_Y, 320, FOOTER_H, DisplayManager::Color::DARK_GRAY);
    display_.drawRect(0, FOOTER_Y, 320, FOOTER_H, DisplayManager::Color::WHITE);

    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::DARK_GRAY);

    // Status ringkas berdasarkan state
    const char *state_str = getStateName_(sync_mgr.getState());
    display_.setCursor(6, FOOTER_Y + (FOOTER_H > 16 ? 8 : 4));
    display_.print(state_str);

    // FUEL (jika ada; placeholder)
    display_.setCursor(220, FOOTER_Y + (FOOTER_H > 16 ? 8 : 4));
    display_.print("FUEL: --.- L");
}

void UIScreen::renderSyncLossScreen_(const SyncManager &sync_mgr,
                                    const UIStateMachine &ui_state) {
    // Full screen override untuk SYNC_LOSS
    DisplayManager::Color bg = ui_state.shouldBlinkOn() ? DisplayManager::Color::RED : DisplayManager::Color::BLACK;
    display_.fillScreen(bg);
    
    // Large warning text
    display_.setTextSize(2);
    display_.setTextColor(DisplayManager::Color::WHITE, bg);
    
    display_.printCentered(80, "SYNC LOSS", DisplayManager::Color::WHITE, bg, 2);
    display_.printCentered(110, "NO ECU", DisplayManager::Color::WHITE, bg, 1);
    
    // Recovery info
    uint8_t progress = sync_mgr.getRecoveryProgress();
    display_.setTextSize(1);
    display_.setCursor(80, 150);
    display_.printf("Recovery: %d%%", progress);
    
    // Countdown timer
    uint32_t elapsed = sync_mgr.getStateElapsedTime();
    uint32_t countdown = (2000 - elapsed) / 100;  // 2-second recovery
    display_.setCursor(100, 170);
    display_.printf("T: %lu", countdown);
}

DisplayManager::Color UIScreen::getStateColor_(SyncManager::SyncState state) const {
    switch (state) {
        case SyncManager::SyncState::NO_DATA:  return DisplayManager::Color::AMBER;
        case SyncManager::SyncState::NORMAL:    return DisplayManager::Color::GREEN;
        case SyncManager::SyncState::CAUTION:   return DisplayManager::Color::AMBER;
        case SyncManager::SyncState::WARNING:   return DisplayManager::Color::RED;
        case SyncManager::SyncState::SYNC_LOSS: return DisplayManager::Color::RED;
        case SyncManager::SyncState::RECOVERY:  return DisplayManager::Color::AMBER;
        default:                                 return DisplayManager::Color::WHITE;
    }
}

const char* UIScreen::getStateName_(SyncManager::SyncState state) const {
    switch (state) {
        case SyncManager::SyncState::NO_DATA:  return "NO DATA";
        case SyncManager::SyncState::NORMAL:    return "NORMAL";
        case SyncManager::SyncState::CAUTION:   return "CAUTION";
        case SyncManager::SyncState::WARNING:   return "WARNING";
        case SyncManager::SyncState::SYNC_LOSS: return "SYNC LOSS";
        case SyncManager::SyncState::RECOVERY:  return "RECOVERY";
        default:                                 return "UNKNOWN";
    }
}

void UIScreen::drawBox_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                        DisplayManager::Color border, DisplayManager::Color fill) {
    display_.drawBox(x, y, w, h, border, fill);
}

void UIScreen::drawParameterBox_(uint16_t x, uint16_t y, const char *label,
                                const char *value, DisplayManager::Color fg) {
    display_.drawParameterBox(x, y, label, value, fg);
}

void UIScreen::drawGridCell_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                             const char* label, const char* value,
                             DisplayManager::Color labelColor,
                             DisplayManager::Color valueColor,
                             DisplayManager::Color borderColor,
                             uint8_t valueSize) {
    // Box
    DisplayManager::Color border = borderColor;
    DisplayManager::Color fill = DisplayManager::Color::DARK_GRAY;
    display_.drawBox(x, y, w, h, border, fill);

    // Label (kiri atas)
    display_.setTextSize(1);
    display_.setTextColor(labelColor, fill);
    display_.setCursor(x + 6, y + 6);
    display_.print(label);

    // Value (besar, center-ish)
    display_.setTextSize(valueSize);
    display_.setTextColor(valueColor, fill);
    // Perkiraan posisi tengah vertikal
    uint16_t vy = y + (h / 2) - (valueSize * 4);
    // Horizontal: kira2 offset agar tidak melebihi kotak
    uint16_t vx = x + 6;
    display_.setCursor(vx, vy);
    display_.print(value);
}

const char* UIScreen::headerEcuStatusText_(const ECUData &ecu_data, const SyncManager &sync_mgr) const {
    // Tanpa menambah state publik baru: interpretasi BOOT/WAIT via timestamp/data
    if (ecu_data.lastUpdateMillis == 0) return "BOOT";   // pertama kali nyala
    if (!ecu_data.isDataValid) return "WAIT";            // belum ada data valid
    if (!ecu_data.isSynced) return "UNSYNC";             // data ada tapi belum sync
    if (sync_mgr.getState() == SyncManager::SyncState::SYNC_LOSS) return "LOSS";
    return "OK";
}

DisplayManager::Color UIScreen::valueColorForState_(SyncManager::SyncState state) const {
    switch (state) {
        case SyncManager::SyncState::WARNING:
        case SyncManager::SyncState::SYNC_LOSS:
            return DisplayManager::Color::RED;
        case SyncManager::SyncState::CAUTION:
        case SyncManager::SyncState::RECOVERY:
        case SyncManager::SyncState::NO_DATA:
            return DisplayManager::Color::AMBER;
        case SyncManager::SyncState::NORMAL:
        default:
            return DisplayManager::Color::GREEN;
    }
}

DisplayManager::Color UIScreen::borderColorForState_(SyncManager::SyncState state) const {
    switch (state) {
        case SyncManager::SyncState::WARNING:
        case SyncManager::SyncState::SYNC_LOSS:
            return DisplayManager::Color::RED;
        case SyncManager::SyncState::CAUTION:
        case SyncManager::SyncState::RECOVERY:
        case SyncManager::SyncState::NO_DATA:
            return DisplayManager::Color::AMBER;
        case SyncManager::SyncState::NORMAL:
        default:
            return DisplayManager::Color::GREEN;
    }
}

// ===== Fullscreen state renderers =====
void UIScreen::renderBootSelfTest_() {
    // Simple boot/self-test page
    display_.fillScreen(DisplayManager::Color::LIGHT_GRAY);
    display_.printCentered(80, "BOOT", DisplayManager::Color::BLACK, DisplayManager::Color::LIGHT_GRAY, 2);
    display_.printCentered(110, "SELF TEST", DisplayManager::Color::BLACK, DisplayManager::Color::LIGHT_GRAY, 1);
}

void UIScreen::renderWaitECU_() {
    // Waiting for ECU data
    display_.fillScreen(DisplayManager::Color::BLACK);
    display_.printCentered(100, "WAIT ECU", DisplayManager::Color::AMBER, DisplayManager::Color::BLACK, 2);
}

void UIScreen::renderSyncing_() {
    // Data present but not yet synced
    display_.fillScreen(DisplayManager::Color::BLACK);
    display_.printCentered(100, "SYNCING", DisplayManager::Color::CYAN, DisplayManager::Color::BLACK, 2);
}

void UIScreen::renderRecovery_() {
    // Recovery stabilization screen
    display_.fillScreen(DisplayManager::Color::DARK_GRAY);
    display_.printCentered(100, "RECOVERY", DisplayManager::Color::AMBER, DisplayManager::Color::DARK_GRAY, 2);
}

// ===== Annotation helpers =====
void UIScreen::annotateClt_(char *buf, size_t bufSize, int16_t clt, const SyncManager &sync_mgr) const {
    const auto &th = sync_mgr.getThresholds();
    // Basic formatting: value in Celsius
    char tmp[16];
    snprintf(tmp, sizeof(tmp), "%dC", clt);

    // Determine severity based on thresholds
    bool caution = false;
    bool warning = false;
    if (clt > th.clt_max) {
        caution = true;
        if (clt >= th.clt_max + 10) warning = true; // 10C above max => warning
    } else if (clt < th.clt_min) {
        caution = true;
        if (clt <= th.clt_min - 10) warning = true; // 10C below min => warning
    }

    // Compose annotated string
    if (warning) {
        snprintf(buf, bufSize, "%s!!", tmp);
    } else if (caution) {
        snprintf(buf, bufSize, "%s!", tmp);
    } else {
        snprintf(buf, bufSize, "%s", tmp);
    }
}

void UIScreen::annotateAfr_(char *buf, size_t bufSize, uint16_t afr, const SyncManager &sync_mgr) const {
    const auto &th = sync_mgr.getThresholds();
    // Format AFR as XX.XX (100x)
    float afr_val = afr / 100.0f;
    char tmp[16];
    // Keep one decimal for compactness (e.g., 14.7)
    snprintf(tmp, sizeof(tmp), "%.1f", afr_val);

    // Determine severity
    bool caution = false;
    bool warning = false;
    if (afr > th.afr_max) {
        caution = true;
        if (afr >= th.afr_max + 200) warning = true; // > +2.00 extremely lean
    } else if (afr < th.afr_min) {
        caution = true;
        if (afr <= th.afr_min - 200) warning = true; // < -2.00 extremely rich
    }

    if (warning) {
        snprintf(buf, bufSize, "%s!!", tmp);
    } else if (caution) {
        snprintf(buf, bufSize, "%s!", tmp);
    } else {
        snprintf(buf, bufSize, "%s", tmp);
    }
}
