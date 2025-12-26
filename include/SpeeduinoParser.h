#ifndef SPEEDUINO_PARSER_H
#define SPEEDUINO_PARSER_H

#include <Arduino.h>
#include <stdint.h>
#include "ECUData.h"

/**
 * @class SpeeduinoParser
 * @brief Serial parser untuk Speeduino frame (binary, offset-based)
 * 
 * Format:
 * - Header: 0xAA (1 byte)
 * - Data: ~127 byte (offset-based positioning)
 * - Total frame: ~128 byte
 * 
 * Parser adalah passive listener (read-only), tidak mengirim data ke ECU.
 */
class SpeeduinoParser {
public:
    // Frame structure offsets (berdasarkan Speeduino v20+ binary format)
    static constexpr uint8_t FRAME_HEADER = 0xAA;
    static constexpr uint16_t FRAME_SIZE = 128;
    static constexpr uint8_t MAX_CONSECUTIVE_ERRORS = 10;
    
    // Data offsets untuk Speeduino primary realtime response (74 bytes, no header)
    // Based on Speeduino 3.5+ format per TurboMarian example
    static constexpr uint8_t OFFSET_MAP_LO = 4;        // bytes 4-5, little-endian (kPa)
    static constexpr uint8_t OFFSET_MAP_HI = 5;
    static constexpr uint8_t OFFSET_IAT = 6;           // 1 byte (signed, °C)
    static constexpr uint8_t OFFSET_CLT = 7;           // 1 byte (signed, °C)
    static constexpr uint8_t OFFSET_BATTERY = 9;       // 1 byte (0.1V per unit)
    static constexpr uint8_t OFFSET_AFR = 10;          // 1 byte (AFR/10, e.g., 147 = 14.7)
    static constexpr uint8_t OFFSET_RPM_LO = 14;       // bytes 14-15, little-endian
    static constexpr uint8_t OFFSET_RPM_HI = 15;
    static constexpr uint8_t OFFSET_ADV = 23;          // 1 byte (advance/ignition)
    static constexpr uint8_t OFFSET_TPS = 24;          // 1 byte (TPS %)
    static constexpr uint8_t OFFSET_FLEX = 34;         // 1 byte (flex fuel %)
    static constexpr uint16_t PRIMARY_RESPONSE_SIZE = 74;  // Primary realtime response size
    
    SpeeduinoParser(uint32_t serial_baud = 115200);
    
    // Initialize serial communication
    void begin(HardwareSerial &serial = Serial);
    
    // Optional: enable primary request mode (active polling)
    // Sends a single-byte command periodically to request realtime data
    void configureRequest(uint8_t cmd, uint32_t period_ms);
    
    // Non-blocking frame parsing dari serial buffer
    bool update(ECUData &ecu_data);
    
    // Get parser statistics
    uint32_t getFramesReceived() const { return frames_received_; }
    uint32_t getFramesErrored() const { return frames_errored_; }
    uint32_t getSyncLosses() const { return sync_losses_; }
    uint32_t getRawBytes() const { return raw_bytes_; }
    uint32_t getLastRxMillis() const { return last_rx_ms_; }
    const char* getLastLine() const { return last_line_[0] ? last_line_ : ""; }
    
    // Debug output
    void debugPrint() const;

private:
    HardwareSerial *serial_;
    uint32_t baud_rate_;
    
    // Parser state machine
    enum class ParserState {
        IDLE,
        HEADER_FOUND,
        COLLECTING_DATA,
        FRAME_READY
    };
    
    ParserState state_;
    uint8_t frame_buffer_[FRAME_SIZE];
    uint16_t buffer_index_;
    uint8_t consecutive_errors_;

    // Generic Fixed CSV line buffer (secondary serial)
    static constexpr uint16_t LINE_BUFFER_SIZE = 160;
    char line_buffer_[LINE_BUFFER_SIZE];
    uint16_t line_index_;
    bool line_ready_;
    char last_line_[LINE_BUFFER_SIZE];
    
    // Statistics
    uint32_t frames_received_;
    uint32_t frames_errored_;
    uint32_t sync_losses_;
    uint8_t valid_streak_ = 0; // consecutive valid frames
    uint32_t raw_bytes_ = 0;    // raw bytes observed on serial
    uint32_t last_rx_ms_ = 0;   // last time a byte was seen

    // Primary request mode
    bool request_mode_ = false;
    uint8_t request_cmd_ = 'r';
    uint32_t request_period_ms_ = 50;
    uint32_t last_request_ms_ = 0;
    bool expect_primary_ = false;      // after sending request, expect 74-byte primary
    uint32_t primary_expect_deadline_ = 0; // timeout for expecting primary response

    // Debug: ring buffer of last received bytes
    static constexpr uint8_t LAST_BYTES_CAP = 64;
    uint8_t last_bytes_[LAST_BYTES_CAP];
    uint8_t last_bytes_idx_ = 0;
    
    // Private parsing methods
    bool tryReadFrame_();
    bool validateFrame_() const;
    void extractDataFromFrame_(ECUData &ecu_data);
    void resetParserState_();

    // Secondary Serial Generic Fixed (CSV) helpers
    void resetLineBuffer_();
    bool tryReadGenericFixedLine_();
    bool extractDataFromGenericFixed_(ECUData &ecu_data, const char *line);
    bool extractDataFromKeyValue_(ECUData &ecu_data, const char *line);
};

#endif
