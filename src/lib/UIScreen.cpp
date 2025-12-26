#include "UIScreen.h"

UIScreen::UIScreen(DisplayManager &display)
    : display_(display) {
}

void UIScreen::render(const ECUData &ecu_data,
                      const SyncManager &sync_mgr,
                      const UIStateMachine &ui_state) {
    
    // SYNC_LOSS override: full-screen warning
    if (sync_mgr.getState() == SyncManager::SyncState::SYNC_LOSS) {
        // Redraw only when FULL_SCREEN slice is dirty (blink toggle/state change)
        if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
            renderSyncLossScreen_(sync_mgr, ui_state);
        }
        return;
    }

    // BOOT: perangkat baru menyala, belum ada data
    if (ecu_data.lastUpdateMillis == 0) {
        if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
            renderBootSelfTest_();
        }
        return;
    }

    // WAIT ECU / NO DATA: belum ada frame valid
    if (!ecu_data.isDataValid) {
        if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
            renderWaitECU_();
        }
        return;
    }

    // SYNCING: data masuk tapi belum sinkron
    if (!ecu_data.isSynced) {
        if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
            renderSyncing_();
        }
        return;
    }

    // RECOVERY: masa pemulihan, tampilkan layar khusus
    if (sync_mgr.getState() == SyncManager::SyncState::RECOVERY) {
        if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
            renderRecovery_();
        }
        return;
    }
    
    // Normal rendering dengan dirty-flag optimization
    updateSmooth_(ecu_data);
    // If fullscreen was dirty (e.g., coming from SYNC_LOSS/RECOVERY/WAIT/BOOT),
    // ensure we force values to redraw by clearing previous cache.
    if (ui_state.isSliceDirty(UIStateMachine::UISlice::FULL_SCREEN)) {
        for (int i = 0; i < 6; ++i) {
            prevVals_[i][0] = '\0';
        }
    }
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
    // Header bar: ECU | SYNC | BAT — dengan garis pemisah
    display_.fillRect(0, HEADER_Y, 320, HEADER_H, DisplayManager::Color::BLACK);
    display_.drawLine(0, HEADER_Y + HEADER_H - 1, 319, HEADER_Y + HEADER_H - 1, DisplayManager::Color::WHITE);

    // Tiga kolom seimbang
    const uint16_t colW = 106; // ~320/3
    const uint16_t padX = 6;
    const uint16_t baseY = HEADER_Y + 4;

    // ECU status (kiri)
    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(padX, baseY);
    display_.print("ECU");
    display_.setCursor(padX + 30, baseY);
    const char* ecuTxt = headerEcuStatusText_(ecu_data, sync_mgr);
    DisplayManager::Color ecuCol = ecu_data.isDataValid ? DisplayManager::Color::GREEN : DisplayManager::Color::AMBER;
    display_.setTextColor(ecuCol, DisplayManager::Color::BLACK);
    display_.print(ecuTxt);

    // Jika RECOVERY, tampilkan progress kecil di sebelahnya
    if (sync_mgr.getState() == SyncManager::SyncState::RECOVERY) {
        display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::BLACK);
        display_.setCursor(padX + 65, baseY);
        char recStr[12];
        snprintf(recStr, sizeof(recStr), "%d%%", sync_mgr.getRecoveryProgress());
        display_.print(recStr);
    }

    // SYNC (tengah)
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(colW + padX, baseY);
    display_.print("SYNC");
    display_.setCursor(colW + padX + 36, baseY);
    char syncStr[16];
    snprintf(syncStr, sizeof(syncStr), "%d", ecu_data.syncLossCounter);
    display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::BLACK);
    display_.print(syncStr);

    // BAT (kanan)
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(colW*2 + padX, baseY);
    display_.print("BAT");
    display_.setCursor(colW*2 + padX + 28, baseY);
    char batStr[16];
    snprintf(batStr, sizeof(batStr), "%.1fV", ecu_data.battery / 1000.0);
    // Warna hanya untuk status: nilai tetap putih
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.print(batStr);
}

void UIScreen::renderRPMField_(const ECUData &ecu_data,
                              const SyncManager &sync_mgr) {
    // Row-1, Col-1: RPM (cell pertama, atas)
    int rpmDisp = (int)(smooth_.rpm + 0.5f);
    char rpmStr[16]; snprintf(rpmStr, sizeof(rpmStr), "%d", rpmDisp);
    DisplayManager::Color valCol = valueColorForState_(sync_mgr.getState());
    if (sync_mgr.getState() == SyncManager::SyncState::NO_DATA || !ecu_data.isDataValid) {
        strcpy(rpmStr, "--");
        valCol = DisplayManager::Color::AMBER;
    }
    // Skip redraw if unchanged
    if (strcmp(prevVals_[0], rpmStr) != 0) {
        strncpy(prevVals_[0], rpmStr, sizeof(prevVals_[0]) - 1);
        prevVals_[0][sizeof(prevVals_[0]) - 1] = '\0';
        drawGridCell_(0, GRID_ROW1_Y, CELL_W, GRID_ROW1_H,
                      "RPM", rpmStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }
}

void UIScreen::renderEngineCore_(const ECUData &ecu_data,
                                const SyncManager &sync_mgr) {
    // Row-1, Col-2: MAP
    int mapDisp = (int)(smooth_.map + 0.5f);
    char mapStr[16]; snprintf(mapStr, sizeof(mapStr), "%d", mapDisp);
    DisplayManager::Color valCol = valueColorForState_(sync_mgr.getState());
    if (sync_mgr.getState() == SyncManager::SyncState::NO_DATA || !ecu_data.isDataValid) {
        strcpy(mapStr, "--");
        valCol = DisplayManager::Color::AMBER;
    }
    if (strcmp(prevVals_[1], mapStr) != 0) {
        strncpy(prevVals_[1], mapStr, sizeof(prevVals_[1]) - 1);
        prevVals_[1][sizeof(prevVals_[1]) - 1] = '\0';
        drawGridCell_(CELL_W, GRID_ROW1_Y, CELL_W, GRID_ROW1_H,
                      "MAP", mapStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }

    // Row-1, Col-3: CLT
    char cltStr[16];
    if (!ecu_data.isDataValid) {
        strcpy(cltStr, "--");
        valCol = DisplayManager::Color::AMBER;
    } else {
        annotateClt_(cltStr, sizeof(cltStr), (int16_t)(smooth_.clt + 0.5f), sync_mgr);
        valCol = valueColorForState_(sync_mgr.getState());
    }
    if (strcmp(prevVals_[2], cltStr) != 0) {
        strncpy(prevVals_[2], cltStr, sizeof(prevVals_[2]) - 1);
        prevVals_[2][sizeof(prevVals_[2]) - 1] = '\0';
        drawGridCell_(CELL_W * 2, GRID_ROW1_Y, CELL_W, GRID_ROW1_H,
                      "CLT", cltStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }
}

void UIScreen::renderControlData_(const ECUData &ecu_data,
                                 const SyncManager &sync_mgr) {
    // Row-2, Col-1: IAT
    int iatDisp = (int)(smooth_.iat + 0.5f);
    char iatStr[16]; snprintf(iatStr, sizeof(iatStr), "%d", iatDisp);
    DisplayManager::Color valCol = valueColorForState_(sync_mgr.getState());
    if (!ecu_data.isDataValid) {
        strcpy(iatStr, "--");
        valCol = DisplayManager::Color::AMBER;
    }
    if (strcmp(prevVals_[3], iatStr) != 0) {
        strncpy(prevVals_[3], iatStr, sizeof(prevVals_[3]) - 1);
        prevVals_[3][sizeof(prevVals_[3]) - 1] = '\0';
        drawGridCell_(0, GRID_ROW2_Y, CELL_W, GRID_ROW2_H,
                      "IAT", iatStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }

    // Row-2, Col-2: AFR
    char afrStr[16];
    if (!ecu_data.isDataValid) {
        strcpy(afrStr, "--");
        valCol = DisplayManager::Color::AMBER;
    } else {
        annotateAfr_(afrStr, sizeof(afrStr), (uint16_t)(smooth_.afr_x100 + 0.5f), sync_mgr);
        valCol = valueColorForState_(sync_mgr.getState());
    }
    if (strcmp(prevVals_[4], afrStr) != 0) {
        strncpy(prevVals_[4], afrStr, sizeof(prevVals_[4]) - 1);
        prevVals_[4][sizeof(prevVals_[4]) - 1] = '\0';
        drawGridCell_(CELL_W, GRID_ROW2_Y, CELL_W, GRID_ROW2_H,
                      "AFR", afrStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }

    // Row-2, Col-3: TPS
    int tpsDisp = (int)(smooth_.tps + 0.5f);
    char tpsStr[16]; snprintf(tpsStr, sizeof(tpsStr), "%d", tpsDisp);
    valCol = valueColorForState_(sync_mgr.getState());
    if (!ecu_data.isDataValid) {
        strcpy(tpsStr, "--");
        valCol = DisplayManager::Color::AMBER;
    }
    if (strcmp(prevVals_[5], tpsStr) != 0) {
        strncpy(prevVals_[5], tpsStr, sizeof(prevVals_[5]) - 1);
        prevVals_[5][sizeof(prevVals_[5]) - 1] = '\0';
        drawGridCell_(CELL_W * 2, GRID_ROW2_Y, CELL_W, GRID_ROW2_H,
                      "TPS", tpsStr,
                      DisplayManager::Color::WHITE,
                      valCol,
                      borderColorForState_(sync_mgr.getState()),
                      3);
    }
}

void UIScreen::renderFooter_(const ECUData &ecu_data,
                            const SyncManager &sync_mgr,
                            const UIStateMachine &ui_state) {
    // Footer: Status atau FUEL x.xx L (placeholder) — dengan garis pemisah
    display_.fillRect(0, FOOTER_Y, 320, FOOTER_H, DisplayManager::Color::BLACK);
    display_.drawLine(0, FOOTER_Y, 319, FOOTER_Y, DisplayManager::Color::WHITE);

    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);

    // Status ringkas berdasarkan state
    const char *state_str = getStateName_(sync_mgr.getState());
    display_.setCursor(6, FOOTER_Y + 4);
    display_.print(state_str);

    // FUEL (jika ada; placeholder)
    display_.setCursor(220, FOOTER_Y + 4);
    display_.print("FUEL: --.- L");
}

void UIScreen::renderSyncLossScreen_(const SyncManager &sync_mgr,
                                    const UIStateMachine &ui_state) {
    // Full screen override untuk SYNC_LOSS
    DisplayManager::Color bg = ui_state.shouldBlinkOn() ? DisplayManager::Color::RED : DisplayManager::Color::BLACK;
    display_.fillScreen(bg);
    
    // Large warning text — maksimalkan ukuran
    display_.setTextSize(4);
    display_.setTextColor(DisplayManager::Color::WHITE, bg);
    
    display_.printCentered(50, "SYNC", DisplayManager::Color::WHITE, bg, 4);
    display_.printCentered(100, "LOSS", DisplayManager::Color::WHITE, bg, 4);
    display_.printCentered(150, "NO ECU", DisplayManager::Color::WHITE, bg, 3);
    
    // Recovery info — lebih besar dan seimbang
    uint8_t progress = sync_mgr.getRecoveryProgress();
    display_.setTextSize(2);
    display_.setTextColor(DisplayManager::Color::AMBER, bg);
    char recStr[20];
    snprintf(recStr, sizeof(recStr), "Recovery: %d%%", progress);
    display_.printCentered(195, recStr, DisplayManager::Color::AMBER, bg, 2);
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
    // Clean avionics style: no borders, just spacing and color coding
    DisplayManager::Color fill = DisplayManager::Color::BLACK;
    // Reduce flicker: clear only the value area, keep label area intact
    const uint16_t labelAreaH = 20; // approx height for label line
    display_.fillRect(x, y + labelAreaH, w, (h > labelAreaH ? h - labelAreaH : 0), fill);

    // Label (kiri atas, kecil)
    display_.setFont(nullptr);
    display_.setTextSize(1);
    display_.setTextColor(labelColor, fill);
    display_.setCursor(x + 6, y + 4);
    display_.print(label);

#if USE_FREEFONT
    // Mega 2560: gunakan FreeFont untuk smooth rendering
    uint16_t len = strlen(value);
    const GFXfont *font = &FreeSansBold12pt7b;
    
    if (len <= 3 && w >= 100) {
        font = &FreeSansBold18pt7b;
    } else if (len <= 6) {
        font = &FreeSansBold12pt7b;
    } else {
        font = &FreeSans9pt7b;
    }
    
    display_.setFont(font);
    display_.setTextColor(valueColor, fill);
    
    int16_t bx, by;
    uint16_t bw, bh;
    display_.getTFT()->getTextBounds(value, 0, 0, &bx, &by, &bw, &bh);
    
    uint16_t vx = x + (w > bw ? (w - bw) / 2 : 2);
    uint16_t vy = y + h / 2 + bh / 2;
    
    display_.setCursor(vx, vy);
    display_.print(value);
    display_.setFont(nullptr);
#else
    // UNO: gunakan built-in font dengan auto-sizing
    uint8_t desired = valueSize;
    uint16_t len = strlen(value);
    uint8_t maxByW = (len > 0) ? (uint8_t)(w / (6 * len)) : desired;
    uint8_t maxByH = (uint8_t)(h / 8);
    uint8_t finalSize = desired;
    if (maxByW == 0) maxByW = 1;
    if (maxByH == 0) maxByH = 1;
    if (finalSize > maxByW) finalSize = maxByW;
    if (finalSize > maxByH) finalSize = maxByH;
    if (finalSize < 1) finalSize = 1;

    display_.setTextSize(finalSize);
    display_.setTextColor(valueColor, fill);
    uint16_t charW = 6 * finalSize;
    uint16_t charH = 8 * finalSize;
    uint16_t textW = len * charW;
    uint16_t vx = x + (w > textW ? (w - textW) / 2 : 2);
    uint16_t vy = y + (h > charH ? (h - charH) / 2 : 2);
    display_.setCursor(vx, vy);
    display_.print(value);
#endif
}

// Draw only the value area (used for potential future granular updates)
void UIScreen::drawGridValueArea_(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                  const char* value,
                                  DisplayManager::Color valueColor,
                                  uint8_t valueSize) {
    DisplayManager::Color fill = DisplayManager::Color::BLACK;
    const uint16_t labelAreaH = 20;
    display_.fillRect(x, y + labelAreaH, w, (h > labelAreaH ? h - labelAreaH : 0), fill);
#if USE_FREEFONT
    const GFXfont *font = &FreeSansBold12pt7b;
    uint16_t len = strlen(value);
    if (len <= 3 && w >= 100) font = &FreeSansBold18pt7b;
    else if (len <= 6) font = &FreeSansBold12pt7b;
    else font = &FreeSans9pt7b;
    display_.setFont(font);
    display_.setTextColor(valueColor, fill);
    int16_t bx, by; uint16_t bw, bh;
    display_.getTFT()->getTextBounds(value, 0, 0, &bx, &by, &bw, &bh);
    uint16_t vx = x + (w > bw ? (w - bw) / 2 : 2);
    uint16_t vy = y + h / 2 + bh / 2;
    display_.setCursor(vx, vy);
    display_.print(value);
    display_.setFont(nullptr);
#else
    uint8_t finalSize = valueSize;
    uint16_t len = strlen(value);
    uint8_t maxByW = (len > 0) ? (uint8_t)(w / (6 * len)) : finalSize;
    uint8_t maxByH = (uint8_t)(h / 8);
    if (maxByW == 0) maxByW = 1; if (maxByH == 0) maxByH = 1;
    if (finalSize > maxByW) finalSize = maxByW; if (finalSize > maxByH) finalSize = maxByH;
    display_.setTextSize(finalSize);
    display_.setTextColor(valueColor, fill);
    uint16_t charW = 6 * finalSize; uint16_t charH = 8 * finalSize;
    uint16_t textW = len * charW;
    uint16_t vx = x + (w > textW ? (w - textW) / 2 : 2);
    uint16_t vy = y + (h > charH ? (h - charH) / 2 : 2);
    display_.setCursor(vx, vy);
    display_.print(value);
#endif
}

// ===== Smoothing helpers =====
void UIScreen::updateSmooth_(const ECUData &ecu) {
    // Parameter-specific smoothing factors (higher = faster response)
    const float alpha_rpm = 0.35f;
    const float alpha_map = 0.30f;
    const float alpha_clt = 0.15f;
    const float alpha_iat = 0.15f;
    const float alpha_afr = 0.25f;
    const float alpha_tps = 0.35f;
    const float alpha_bat = 0.10f;
    if (!smooth_.initialized) {
        smooth_.rpm = ecu.rpm;
        smooth_.map = ecu.map;
        smooth_.clt = ecu.clt;
        smooth_.iat = ecu.iat;
        smooth_.afr_x100 = ecu.afr;
        smooth_.tps = ecu.tps;
        smooth_.battery = ecu.battery;
        smooth_.initialized = true;
        return;
    }
    smooth_.rpm = lerp_(smooth_.rpm, (float)ecu.rpm, alpha_rpm);
    smooth_.map = lerp_(smooth_.map, (float)ecu.map, alpha_map);
    smooth_.clt = lerp_(smooth_.clt, (float)ecu.clt, alpha_clt);
    smooth_.iat = lerp_(smooth_.iat, (float)ecu.iat, alpha_iat);
    smooth_.afr_x100 = lerp_(smooth_.afr_x100, (float)ecu.afr, alpha_afr);
    smooth_.tps = lerp_(smooth_.tps, (float)ecu.tps, alpha_tps);
    smooth_.battery = lerp_(smooth_.battery, (float)ecu.battery, alpha_bat);
}

float UIScreen::lerp_(float from, float to, float alpha) {
    return from + alpha * (to - from);
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
    // Avionics-style self-test page
    display_.fillScreen(DisplayManager::Color::BLACK);

    // Framed box
    const uint16_t x = 20, y = 30, w = 280, h = 180;
    display_.drawBox(x, y, w, h, DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);

    // Title
    display_.setTextSize(2);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(x + 10, y + 12);
    display_.print("ECU MONITOR");

    // Items
    uint16_t lineY = y + 50;
    display_.setTextSize(2);
    display_.setCursor(x + 10, lineY);
    display_.print("TFT   ");
    display_.setTextColor(DisplayManager::Color::GREEN, DisplayManager::Color::BLACK);
    display_.print("OK");

    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(x + 10, lineY + 30);
    display_.print("MCU   ");
    display_.setTextColor(DisplayManager::Color::GREEN, DisplayManager::Color::BLACK);
    display_.print("OK");

    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(x + 10, lineY + 60);
    display_.print("BUS   ");
    display_.setTextColor(DisplayManager::Color::GREEN, DisplayManager::Color::BLACK);
    display_.print("OK");

    // Footer INIT... — static text, tidak blink
    display_.setTextSize(2);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(x + 10, y + h - 28);
    display_.print("INIT...");
}

void UIScreen::renderWaitECU_() {
    // Honest wait screen: ECU belum kirim data
    display_.fillScreen(DisplayManager::Color::BLACK);

    const uint16_t x = 20, y = 30, w = 280, h = 160;
    display_.drawBox(x, y, w, h, DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);

    // Header
    display_.setTextSize(2);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.setCursor(x + 10, y + 12);
    display_.print("ECU MONITOR");

    // Status lines
    display_.setTextSize(2);
    display_.setCursor(x + 10, y + 50);
    display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::BLACK);
    display_.print("WAIT ECU");

    display_.setTextSize(1);
    display_.setCursor(x + 10, y + 85);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.print("SERIAL LINK");

    // Placeholders row
    display_.setCursor(x + 10, y + h - 40);
    display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
    display_.print("RPM ---   CLT ---   AFR --");
}

void UIScreen::renderSyncing_() {
    // Data masuk, belum stabil — layout 3x2 grid seperti normal mode
    display_.fillScreen(DisplayManager::Color::BLACK);

    // Header
    display_.fillRect(0, 0, 320, 20, DisplayManager::Color::DARK_GRAY);
    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::CYAN, DisplayManager::Color::DARK_GRAY);
    display_.setCursor(6, 6);
    display_.print("SYNCING");

    // Grid 3x2 dengan placeholder
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 3; col++) {
            uint16_t x = col * CELL_W;
            uint16_t y = 20 + row * 100;
            display_.fillRect(x, y, CELL_W, 100, DisplayManager::Color::BLACK);
            
            display_.setTextSize(1);
            display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
            display_.setCursor(x + 6, y + 4);
            
            if (row == 0 && col == 0) display_.print("RPM");
            else if (row == 0 && col == 1) display_.print("MAP");
            else if (row == 0 && col == 2) display_.print("CLT");
            else if (row == 1 && col == 0) display_.print("IAT");
            else if (row == 1 && col == 1) display_.print("AFR");
            else if (row == 1 && col == 2) display_.print("TPS");
            
            display_.setTextSize(2);
            display_.setCursor(x + 10, y + 45);
            display_.print("--");
        }
    }
}

void UIScreen::renderRecovery_() {
    // Recovery: short delay, avoid flicker & false alarm — layout 3x2 grid
    display_.fillScreen(DisplayManager::Color::BLACK);

    // Header
    display_.fillRect(0, 0, 320, 20, DisplayManager::Color::DARK_GRAY);
    display_.setTextSize(1);
    display_.setTextColor(DisplayManager::Color::AMBER, DisplayManager::Color::DARK_GRAY);
    display_.setCursor(6, 6);
    display_.print("RECOVERING");

    // Grid 3x2 dengan placeholder
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 3; col++) {
            uint16_t x = col * CELL_W;
            uint16_t y = 20 + row * 100;
            display_.fillRect(x, y, CELL_W, 100, DisplayManager::Color::BLACK);
            
            display_.setTextSize(1);
            display_.setTextColor(DisplayManager::Color::WHITE, DisplayManager::Color::BLACK);
            display_.setCursor(x + 6, y + 4);
            
            if (row == 0 && col == 0) display_.print("RPM");
            else if (row == 0 && col == 1) display_.print("MAP");
            else if (row == 0 && col == 2) display_.print("CLT");
            else if (row == 1 && col == 0) display_.print("IAT");
            else if (row == 1 && col == 1) display_.print("AFR");
            else if (row == 1 && col == 2) display_.print("TPS");
            
            display_.setTextSize(2);
            display_.setCursor(x + 10, y + 45);
            display_.print("--");
        }
    }
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
    snprintf(tmp, sizeof(tmp), "%.1f", (double)afr_val);

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
