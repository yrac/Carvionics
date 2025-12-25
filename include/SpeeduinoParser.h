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
    
    // Data offsets dalam frame (contoh Speeduino standard)
    static constexpr uint8_t OFFSET_RPM = 0;           // 2 byte (uint16_t)
    static constexpr uint8_t OFFSET_CLT = 2;           // 1 byte (int8_t)
    static constexpr uint8_t OFFSET_AFR = 4;           // 2 byte (uint16_t)
    static constexpr uint8_t OFFSET_MAP = 6;           // 1 byte (uint8_t)
    static constexpr uint8_t OFFSET_TPS = 10;          // 1 byte (uint8_t)
    static constexpr uint8_t OFFSET_IAT = 14;          // 1 byte (int8_t)
    static constexpr uint8_t OFFSET_BATTERY = 20;      // 1 byte (uint8_t, dalam 0.1V)
    static constexpr uint8_t OFFSET_STATUS = 31;       // 1 byte (status flags)
    
    SpeeduinoParser(uint32_t serial_baud = 115200);
    
    // Initialize serial communication
    void begin(HardwareSerial &serial = Serial);
    
    // Non-blocking frame parsing dari serial buffer
    bool update(ECUData &ecu_data);
    
    // Get parser statistics
    uint32_t getFramesReceived() const { return frames_received_; }
    uint32_t getFramesErrored() const { return frames_errored_; }
    uint32_t getSyncLosses() const { return sync_losses_; }
    
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
    
    // Statistics
    uint32_t frames_received_;
    uint32_t frames_errored_;
    uint32_t sync_losses_;
    
    // Private parsing methods
    bool tryReadFrame_();
    bool validateFrame_() const;
    void extractDataFromFrame_(ECUData &ecu_data);
    void resetParserState_();

    // Secondary Serial Generic Fixed (CSV) helpers
    void resetLineBuffer_();
    bool tryReadGenericFixedLine_();
    bool extractDataFromGenericFixed_(ECUData &ecu_data, const char *line);
};

#endif
