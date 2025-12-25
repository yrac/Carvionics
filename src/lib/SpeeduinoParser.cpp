#include "SpeeduinoParser.h"

SpeeduinoParser::SpeeduinoParser(uint32_t serial_baud)
    : serial_(nullptr),
      baud_rate_(serial_baud),
      state_(ParserState::IDLE),
      buffer_index_(0),
      consecutive_errors_(0),
      frames_received_(0),
      frames_errored_(0),
      sync_losses_(0) {
    memset(frame_buffer_, 0, FRAME_SIZE);
}

void SpeeduinoParser::begin(HardwareSerial &serial) {
    serial_ = &serial;
    serial_->begin(baud_rate_);
    resetParserState_();
}

bool SpeeduinoParser::update(ECUData &ecu_data) {
    if (!serial_) return false;
    
    // Non-blocking read dari serial buffer
    while (serial_->available()) {
        uint8_t byte = serial_->read();
        
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
    }
    
    // Check if frame ready dan validate
    if (state_ == ParserState::FRAME_READY) {
        if (validateFrame_()) {
            extractDataFromFrame_(ecu_data);
            frames_received_++;
            consecutive_errors_ = 0;
            resetParserState_();
            return true;
        } else {
            frames_errored_++;
            consecutive_errors_++;
            
            if (consecutive_errors_ >= MAX_CONSECUTIVE_ERRORS) {
                sync_losses_++;
                ecu_data.syncLossCounter++;
                ecu_data.isSynced = false;
                consecutive_errors_ = 0;
            }
            
            resetParserState_();
            return false;
        }
    }
    
    return false;
}

bool SpeeduinoParser::validateFrame_() const {
    if (buffer_index_ < FRAME_SIZE) return false;
    if (frame_buffer_[0] != FRAME_HEADER) return false;
    
    // Basic sanity check pada offset yang critical
    // RPM range check
    uint16_t rpm = (frame_buffer_[OFFSET_RPM + 1] << 8) | frame_buffer_[OFFSET_RPM];
    if (rpm > 15000) return false;  // Unrealistic RPM
    
    // Battery range check (0..16V)
    uint8_t battery_raw = frame_buffer_[OFFSET_BATTERY];
    if (battery_raw > 160) return false;
    
    return true;
}

void SpeeduinoParser::extractDataFromFrame_(ECUData &ecu_data) {
    // RPM: offset 0, 2 byte (little-endian)
    ecu_data.rpm = (frame_buffer_[OFFSET_RPM + 1] << 8) | frame_buffer_[OFFSET_RPM];
    
    // CLT: offset 2, 1 byte (signed)
    ecu_data.clt = (int8_t)frame_buffer_[OFFSET_CLT];
    
    // AFR: offset 4, 2 byte (little-endian, 100x)
    ecu_data.afr = (frame_buffer_[OFFSET_AFR + 1] << 8) | frame_buffer_[OFFSET_AFR];
    
    // MAP: offset 6, 1 byte
    ecu_data.map = frame_buffer_[OFFSET_MAP];
    
    // TPS: offset 10, 1 byte (0..100)
    ecu_data.tps = frame_buffer_[OFFSET_TPS];
    
    // IAT: offset 14, 1 byte (signed)
    ecu_data.iat = (int8_t)frame_buffer_[OFFSET_IAT];
    
    // Battery: offset 20, 1 byte (0.1V per unit), convert to mV
    ecu_data.battery = frame_buffer_[OFFSET_BATTERY] * 100;
    
    // Status byte untuk sync check
    uint8_t status = frame_buffer_[OFFSET_STATUS];
    ecu_data.isSynced = (status & 0x01) != 0;  // Bit 0 = sync status
    
    // Update timestamps
    ecu_data.lastUpdateMillis = millis();
    ecu_data.isDataValid = true;
}

void SpeeduinoParser::resetParserState_() {
    state_ = ParserState::IDLE;
    buffer_index_ = 0;
}

void SpeeduinoParser::debugPrint() const {
    Serial.println("\n=== SpeeduinoParser Stats ===");
    Serial.print("Frames Received: "); Serial.println(frames_received_);
    Serial.print("Frames Errored: "); Serial.println(frames_errored_);
    Serial.print("Sync Losses: "); Serial.println(sync_losses_);
    Serial.print("Consecutive Errors: "); Serial.println(consecutive_errors_);
}
