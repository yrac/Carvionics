#include "SpeeduinoParser.h"

SpeeduinoParser::SpeeduinoParser(uint32_t serial_baud)
        : serial_(nullptr),
            baud_rate_(serial_baud),
            state_(ParserState::IDLE),
            buffer_index_(0),
            consecutive_errors_(0),
            line_index_(0),
            line_ready_(false),
            frames_received_(0),
            frames_errored_(0),
            sync_losses_(0) {
        memset(frame_buffer_, 0, FRAME_SIZE);
        resetLineBuffer_();
        memset(last_bytes_, 0, LAST_BYTES_CAP);
        last_bytes_idx_ = 0;
}

void SpeeduinoParser::begin(HardwareSerial &serial) {
    serial_ = &serial;
    serial_->begin(baud_rate_);
    resetParserState_();
    // If compiled with request macros, configure defaults
    #ifdef USE_PRIMARY_REQUEST
    request_mode_ = true;
    #ifdef PRIMARY_REQ_CMD
    request_cmd_ = (uint8_t)PRIMARY_REQ_CMD;
    #endif
    #ifdef PRIMARY_REQ_PERIOD_MS
    request_period_ms_ = (uint32_t)PRIMARY_REQ_PERIOD_MS;
    #endif
    #endif
}

bool SpeeduinoParser::update(ECUData &ecu_data) {
    if (!serial_) return false;
    
    // Active polling if enabled
    if (request_mode_) {
        uint32_t now = millis();
        if (now - last_request_ms_ >= request_period_ms_) {
            serial_->write(&request_cmd_, 1);
            last_request_ms_ = now;
            // Expect a primary response shortly after
            expect_primary_ = true;
            primary_expect_deadline_ = now + 150; // 150ms window
        }
    }
    
    // If expecting a primary response (74 bytes, no header), capture aligned buffer
    if (expect_primary_) {
        // Incrementally collect up to PRIMARY_RESPONSE_SIZE bytes
        while (serial_->available() && buffer_index_ < PRIMARY_RESPONSE_SIZE) {
            frame_buffer_[buffer_index_++] = serial_->read();
        }
        if (buffer_index_ >= PRIMARY_RESPONSE_SIZE) {
            state_ = ParserState::FRAME_READY;
            expect_primary_ = false;
        } else if (millis() > primary_expect_deadline_) {
            // timeout window passed; stop expecting primary and reset buffer
            expect_primary_ = false;
            buffer_index_ = 0;
        }
    }

    // Non-blocking read dari serial buffer
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        raw_bytes_++;
        last_rx_ms_ = millis();
        // keep last bytes for debug
        last_bytes_[last_bytes_idx_] = byte;
        last_bytes_idx_ = (last_bytes_idx_ + 1) % LAST_BYTES_CAP;

        switch (state_) {
            case ParserState::IDLE:
                if (byte == FRAME_HEADER) {
                    frame_buffer_[0] = byte;
                    buffer_index_ = 1;
                    state_ = ParserState::HEADER_FOUND;
                }
                break;
            
            case ParserState::HEADER_FOUND:
            case ParserState::COLLECTING_DATA:
                if (buffer_index_ < FRAME_SIZE) {
                    frame_buffer_[buffer_index_++] = byte;
                    state_ = ParserState::COLLECTING_DATA;
                    
                    if (buffer_index_ >= FRAME_SIZE) {
                        state_ = ParserState::FRAME_READY;
                    }
                }
                break;
            
            case ParserState::FRAME_READY:
                // Should not reach here in non-blocking mode
                break;
        }

        // Collect ASCII CSV for Secondary Serial Generic Fixed
        if (byte == '\n' || byte == '\r') {
            if (line_index_ > 0) {
                line_buffer_[line_index_] = '\0';
                line_ready_ = true;
            }
        } else if (line_index_ < LINE_BUFFER_SIZE - 1) {
            if ((byte >= 32 && byte <= 126) || byte == '\t') {
                line_buffer_[line_index_++] = (char)byte;
            }
        } else {
            // Overflow; reset
            resetLineBuffer_();
        }
    }
    
    // Check if frame ready dan validate
    if (state_ == ParserState::FRAME_READY) {
        if (validateFrame_()) {
            extractDataFromFrame_(ecu_data);
            frames_received_++;
            consecutive_errors_ = 0;
            // If binary status bit does not indicate sync, allow
            // a safe fallback after a few good frames with plausible data
            if (!ecu_data.isSynced) {
                valid_streak_++;
                if (valid_streak_ >= 3 && ecu_data.rpm > 0) {
                    ecu_data.isSynced = true; // treat as synced
                }
            } else {
                valid_streak_ = 0;
            }
            resetParserState_();
            resetLineBuffer_();
            return true;
        } else {
            frames_errored_++;
            consecutive_errors_++;
            valid_streak_ = 0;
            
            if (consecutive_errors_ >= MAX_CONSECUTIVE_ERRORS) {
                sync_losses_++;
                ecu_data.syncLossCounter++;
                ecu_data.isSynced = false;
                consecutive_errors_ = 0;
            }
            
            resetParserState_();
            resetLineBuffer_();
            return false;
        }
    }

    // If no binary frame, try ASCII line parsing (key=value or generic CSV)
    if (line_ready_) {
        bool has_eq = false;
        for (uint16_t i = 0; i < line_index_; ++i) { if (line_buffer_[i] == '=') { has_eq = true; break; } }
        bool ok = false;
        if (has_eq) {
            ok = extractDataFromKeyValue_(ecu_data, line_buffer_);
        } else {
            ok = extractDataFromGenericFixed_(ecu_data, line_buffer_);
        }
        resetLineBuffer_();
        if (ok) {
            frames_received_++;
            consecutive_errors_ = 0;
            ecu_data.isSynced = true;
            valid_streak_ = 0;
            return true;
        } else {
            frames_errored_++;
            consecutive_errors_++;
            valid_streak_ = 0;
            if (consecutive_errors_ >= MAX_CONSECUTIVE_ERRORS) {
                sync_losses_++;
                ecu_data.syncLossCounter++;
                ecu_data.isSynced = false;
                consecutive_errors_ = 0;
            }
            return false;
        }
    }
    
    return false;
}

bool SpeeduinoParser::validateFrame_() const {
    // Check for primary response format (74 bytes, no header)
    // or binary frame format (128 bytes, header 0xAA)
    
    if (buffer_index_ >= PRIMARY_RESPONSE_SIZE && frame_buffer_[0] != FRAME_HEADER) {
        // Likely a primary response (no 0xAA header)
        // Basic sanity: check RPM and battery range
        uint16_t rpm = (frame_buffer_[OFFSET_RPM_HI] << 8) | frame_buffer_[OFFSET_RPM_LO];
        if (rpm > 15000) return false;
        
        uint8_t battery_raw = frame_buffer_[OFFSET_BATTERY];
        if (battery_raw > 160) return false;  // > 16V unrealistic
        
        return true;
    } else if (buffer_index_ >= FRAME_SIZE && frame_buffer_[0] == FRAME_HEADER) {
        // Binary frame format (128 bytes with 0xAA header)
        return true;
    }
    
    return false;
}

void SpeeduinoParser::extractDataFromFrame_(ECUData &ecu_data) {
    // Parse primary response format (74 bytes, no header)
    if (buffer_index_ >= PRIMARY_RESPONSE_SIZE && frame_buffer_[0] != FRAME_HEADER) {
        // Primary realtime response (Speeduino 3.5+)
        
        // MAP: offset 4-5, 2 byte little-endian (kPa)
        uint16_t map_raw = (frame_buffer_[OFFSET_MAP_HI] << 8) | frame_buffer_[OFFSET_MAP_LO];
        ecu_data.map = map_raw >> 8;  // Simplify to byte range for display
        
        // CLT: offset 7, 1 byte signed (°C)
        ecu_data.clt = (int8_t)frame_buffer_[OFFSET_CLT];
        
        // IAT: offset 6, 1 byte signed (°C)
        ecu_data.iat = (int8_t)frame_buffer_[OFFSET_IAT];
        
        // Battery: offset 9, 1 byte (0.1V per unit), convert to mV
        ecu_data.battery = frame_buffer_[OFFSET_BATTERY] * 100;
        
        // AFR: offset 10, 1 byte (AFR/10, e.g., 147 = 14.7), scale to 100x
        ecu_data.afr = (uint16_t)frame_buffer_[OFFSET_AFR] * 10;
        
        // RPM: offset 14-15, 2 byte little-endian
        ecu_data.rpm = (frame_buffer_[OFFSET_RPM_HI] << 8) | frame_buffer_[OFFSET_RPM_LO];
        
        // TPS: offset 24, 1 byte (%)
        ecu_data.tps = frame_buffer_[OFFSET_TPS];
        
        ecu_data.isSynced = true;  // Primary response = synced
    } else if (buffer_index_ >= FRAME_SIZE && frame_buffer_[0] == FRAME_HEADER) {
        // Binary frame format (128 bytes) - for backward compatibility
        ecu_data.isSynced = false;
    }
    
    // Update timestamps
    ecu_data.lastUpdateMillis = millis();
    ecu_data.isDataValid = true;
}

void SpeeduinoParser::resetParserState_() {
    state_ = ParserState::IDLE;
    buffer_index_ = 0;
}

void SpeeduinoParser::resetLineBuffer_() {
    line_index_ = 0;
    line_ready_ = false;
    if (LINE_BUFFER_SIZE > 0) line_buffer_[0] = '\0';
}

// Try parse Secondary Serial Generic Fixed CSV line
// Assumption: Fixed-order CSV fields, typical order containing RPM, MAP, TPS, CLT, IAT, AFR, BAT
// This function is resilient and attempts to map plausible indices.
bool SpeeduinoParser::extractDataFromGenericFixed_(ECUData &ecu_data, const char *line) {
    // Tokenize by commas
    // We'll parse up to 24 fields to be safe
    const uint8_t MAX_FIELDS = 24;
    const char *fields[MAX_FIELDS];
    uint8_t count = 0;
    const char *p = line;
    fields[count++] = p;
    while (*p && count < MAX_FIELDS) {
        if (*p == ',') {
            fields[count++] = p + 1;
        }
        p++;
    }
    if (count < 5) {
        // Too few fields to be generic fixed
        return false;
    }

    auto toInt = [](const char *s) -> int32_t {
        // Parse int safely
        bool neg = false;
        int32_t val = 0;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '-') { neg = true; s++; }
        while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
        return neg ? -val : val;
    };

    auto toFloat = [](const char *s) -> float {
        // Simple atof without locale
        while (*s == ' ' || *s == '\t') s++;
        bool neg = false;
        if (*s == '-') { neg = true; s++; }
        int32_t ip = 0; int32_t fp = 0; int32_t div = 1;
        while (*s >= '0' && *s <= '9') { ip = ip * 10 + (*s - '0'); s++; }
        if (*s == '.') { s++; while (*s >= '0' && *s <= '9') { fp = fp * 10 + (*s - '0'); div *= 10; s++; } }
        float v = (float)ip + (fp ? (float)fp / (float)div : 0.0f);
        return neg ? -v : v;
    };

    // Heuristic index mapping: try common order
    // Default indices (adjust if out-of-range):
    int8_t idx_rpm = 0;
    int8_t idx_map = 1;
    int8_t idx_tps = 2;
    int8_t idx_clt = 3;
    int8_t idx_iat = 4;
    int8_t idx_afr = 5;
    int8_t idx_bat = 6;

    // Extract raw values with bounds checking
    auto getField = [&](int8_t idx) -> const char* {
        return (idx >= 0 && idx < (int8_t)count) ? fields[idx] : nullptr;
    };

    const char *s_rpm = getField(idx_rpm);
    const char *s_map = getField(idx_map);
    const char *s_tps = getField(idx_tps);
    const char *s_clt = getField(idx_clt);
    const char *s_iat = getField(idx_iat);
    const char *s_afr = getField(idx_afr);
    const char *s_bat = getField(idx_bat);

    // Minimal requirement: at least one core field present
    if (!s_rpm && !s_map && !s_tps && !s_clt && !s_iat && !s_afr && !s_bat) {
        return false;
    }

    // Compute locals first; assign later only if present
    bool has_rpm = false, has_map = false, has_tps = false, has_clt = false, has_iat = false;
    uint16_t rpm_v = 0, map_v = 0, tps_v = 0; int16_t clt_v = 0, iat_v = 0;
    if (s_rpm) { rpm_v = (uint16_t)toInt(s_rpm); has_rpm = true; }
    if (s_map) { map_v = (uint16_t)toInt(s_map); has_map = true; }
    if (s_tps) { tps_v = (uint16_t)toInt(s_tps); has_tps = true; }
    if (s_clt) { clt_v = (int16_t)toInt(s_clt); has_clt = true; }
    if (s_iat) { iat_v = (int16_t)toInt(s_iat); has_iat = true; }

    // AFR can be float (e.g., 14.7) or 100x (e.g., 1470)
    uint16_t afr100 = 0;
    if (s_afr) {
        float afr_f = toFloat(s_afr);
        if (afr_f > 0.0f && afr_f < 50.0f) {
            afr100 = (uint16_t)(afr_f * 100.0f + 0.5f);
        } else {
            // Treat as 100x if realistic range
            int32_t ai = toInt(s_afr);
            afr100 = (uint16_t)ai;
        }
    }

    // Battery can be float volts or integer tenths; store as mV
    uint16_t bat_mv = 0;
    if (s_bat) {
        float bat_f = toFloat(s_bat);
        if (bat_f > 0.0f && bat_f < 30.0f) {
            bat_mv = (uint16_t)(bat_f * 1000.0f + 0.5f);
        } else {
            int32_t bi = toInt(s_bat);
            // Heuristic: if 0..160 likely tenths of volts
            if (bi >= 0 && bi <= 160) bat_mv = (uint16_t)(bi * 100);
            else if (bi < 30000) bat_mv = (uint16_t)bi; // already millivolts
        }
    }

    // Plausibility checks
    if (has_rpm && rpm_v > 18000) return false;
    if (has_map && map_v > 255) return false;
    if (has_tps && tps_v > 100) return false;
    if (afr100 && (afr100 < 800 || afr100 > 2500)) {
        // If outside reasonable AFR range, keep as unknown
        afr100 = 0;
    }
    if (bat_mv && bat_mv > 20000) bat_mv = 0; // unrealistic

    // Assign to ecu_data
    if (has_rpm) ecu_data.rpm = rpm_v;
    if (has_map) ecu_data.map = map_v;
    if (has_tps) ecu_data.tps = tps_v;
    if (has_clt) ecu_data.clt = clt_v;
    if (has_iat) ecu_data.iat = iat_v;
    if (afr100) ecu_data.afr = afr100; // keep previous if 0
    if (bat_mv) ecu_data.battery = bat_mv;

    ecu_data.lastUpdateMillis = millis();
    ecu_data.isDataValid = true;
    return true;
}

void SpeeduinoParser::debugPrint() const {
    Serial.println("\n=== SpeeduinoParser Stats ===");
    Serial.print("Frames Received: "); Serial.println(frames_received_);
    Serial.print("Frames Errored: "); Serial.println(frames_errored_);
    Serial.print("Sync Losses: "); Serial.println(sync_losses_);
    Serial.print("Consecutive Errors: "); Serial.println(consecutive_errors_);
    Serial.print("Raw Bytes: "); Serial.println(raw_bytes_);
    Serial.print("Millis since last RX: "); Serial.println(last_rx_ms_ ? (millis() - last_rx_ms_) : 0);
    Serial.print("Request Mode: "); Serial.println(request_mode_ ? "ON" : "OFF");
    if (request_mode_) {
        Serial.print("Request Cmd: 0x"); Serial.println(request_cmd_, HEX);
        Serial.print("Request Period(ms): "); Serial.println(request_period_ms_);
    }
    // Print last up to 32 bytes in hex (LSB = most recent)
    Serial.print("Last Bytes (newest last): ");
    uint8_t count = LAST_BYTES_CAP < 32 ? LAST_BYTES_CAP : 32;
    uint8_t start = (last_bytes_idx_ + LAST_BYTES_CAP - count) % LAST_BYTES_CAP;
    for (uint8_t i = 0; i < count; ++i) {
        uint8_t b = last_bytes_[(start + i) % LAST_BYTES_CAP];
        if (b < 16) Serial.print('0');
        Serial.print(b, HEX);
        Serial.print(' ');
    }
    Serial.println();
}

void SpeeduinoParser::configureRequest(uint8_t cmd, uint32_t period_ms) {
    request_mode_ = true;
    request_cmd_ = cmd;
    request_period_ms_ = period_ms ? period_ms : 50;
}

// Parse ASCII line with key=value pairs, tokens separated by comma or space
bool SpeeduinoParser::extractDataFromKeyValue_(ECUData &ecu_data, const char *line) {
    auto toInt = [](const char *s) -> int32_t {
        bool neg = false; int32_t v = 0;
        while (*s == ' ' || *s == '\t') s++;
        if (*s == '-') { neg = true; s++; }
        while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
        return neg ? -v : v;
    };
    auto toFloat = [](const char *s) -> float {
        while (*s == ' ' || *s == '\t') s++;
        bool neg = false; if (*s == '-') { neg = true; s++; }
        int32_t ip = 0, fp = 0, div = 1;
        while (*s >= '0' && *s <= '9') { ip = ip * 10 + (*s - '0'); s++; }
        if (*s == '.') { s++; while (*s >= '0' && *s <= '9') { fp = fp * 10 + (*s - '0'); div *= 10; s++; } }
        float v = (float)ip + (fp ? (float)fp / (float)div : 0.0f);
        return neg ? -v : v;
    };

    // Iterate tokens
    const char *p = line;
    bool any = false;
    while (*p) {
        // Skip separators
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        const char *key_start = p;
        while (*p && *p != '=' && *p != ',' && *p != '\n' && *p != '\r') p++;
        if (*p != '=') { while (*p && *p != ',' && *p != '\n' && *p != '\r') p++; continue; }
        const char *key_end = p; // points at '='
        p++; // move past '='
        const char *val_start = p;
        while (*p && *p != ',' && *p != '\n' && *p != '\r') p++;
        const char *val_end = p;

        // Normalize key to uppercase short id
        char key[12]; uint8_t ki = 0;
        const char *ks = key_start;
        while (ks < key_end && ki < sizeof(key) - 1) {
            char c = *ks++;
            if (c >= 'a' && c <= 'z') c = (char)(c - 32);
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) key[ki++] = c;
        }
        key[ki] = '\0';

        // Extract value string
        char val[24]; uint8_t vi = 0;
        const char *vs = val_start;
        while (vs < val_end && vi < sizeof(val) - 1) { val[vi++] = *vs++; }
        val[vi] = '\0';

        // Map known keys
        if (strcmp(key, "RPM") == 0) { ecu_data.rpm = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "MAP") == 0) { ecu_data.map = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "TPS") == 0) { ecu_data.tps = (uint16_t)toInt(val); any = true; }
        else if (strcmp(key, "CLT") == 0) { ecu_data.clt = (int16_t)toInt(val); any = true; }
        else if (strcmp(key, "IAT") == 0) { ecu_data.iat = (int16_t)toInt(val); any = true; }
        else if (strcmp(key, "AFR") == 0) {
            float f = toFloat(val);
            if (f > 0.0f && f < 50.0f) ecu_data.afr = (uint16_t)(f * 100.0f + 0.5f);
            else {
                int32_t ai = toInt(val);
                if (ai >= 800 && ai <= 2500) ecu_data.afr = (uint16_t)ai;
            }
            any = true;
        }
        else if (strcmp(key, "BAT") == 0 || strcmp(key, "VBAT") == 0 || strcmp(key, "BATTERY") == 0) {
            float f = toFloat(val);
            if (f > 0.0f && f < 30.0f) ecu_data.battery = (uint16_t)(f * 1000.0f + 0.5f);
            else {
                int32_t bi = toInt(val);
                if (bi >= 0 && bi <= 160) ecu_data.battery = (uint16_t)(bi * 100);
            }
            any = true;
        }
        // Advance past separator if present
        if (*p == ',') p++;
    }

    if (any) {
        ecu_data.lastUpdateMillis = millis();
        ecu_data.isDataValid = true;
        return true;
    }
    return false;
}
