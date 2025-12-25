/**
 * DISPLAY CHIP DETECTION SKETCH
 * 
 * Detects TFT display driver IC chip
 * Upload this to identify your display, then use appropriate library
 * 
 * Works with 8-bit parallel TFT shields on Arduino Mega 2560
 */

#include <Arduino.h>

// Pin definitions for 8-bit parallel TFT (MCUFRIEND style)
// Port A = D22-D29 (Data pins 0-7)
// Port C = A0-A7 (Control pins)

#define TFT_RD 14  // PA1
#define TFT_WR 15  // PA2
#define TFT_RS 16  // PA3 (DC/CMD select)
#define TFT_CS 17  // PA4
#define TFT_RST 18 // PA5

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== TFT DISPLAY CHIP DETECTOR ===\n");
    
    // Initialize control pins
    pinMode(TFT_RD, OUTPUT);
    pinMode(TFT_WR, OUTPUT);
    pinMode(TFT_RS, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    
    // Set data pins as inputs initially
    for (int i = 22; i <= 29; i++) {
        pinMode(i, INPUT);
    }
    
    // Perform reset
    Serial.println("Resetting display...");
    digitalWrite(TFT_RST, LOW);
    delay(100);
    digitalWrite(TFT_RST, HIGH);
    delay(500);
    
    // Try to read display ID
    Serial.println("Reading display ID...");
    uint16_t id = readDisplayID();
    
    Serial.print("Display ID: 0x");
    Serial.println(id, HEX);
    
    // Identify chip
    identifyChip(id);
    
    Serial.println("\n=== IDENTIFICATION COMPLETE ===\n");
    
    // Show simple color test
    testColors();
}

void loop() {
    delay(1000);
}

uint16_t readDisplayID() {
    // Set all data pins as inputs
    for (int i = 22; i <= 29; i++) {
        pinMode(i, INPUT);
    }
    
    // Chip select low, read mode
    digitalWrite(TFT_CS, LOW);
    digitalWrite(TFT_RD, LOW);
    digitalWrite(TFT_RS, HIGH);  // Data mode
    
    delay(10);
    
    // Read high byte
    uint16_t id = 0;
    for (int i = 0; i < 8; i++) {
        if (digitalRead(22 + i)) {
            id |= (1 << i);
        }
    }
    
    id <<= 8;
    
    delay(5);
    
    // Read low byte
    for (int i = 0; i < 8; i++) {
        if (digitalRead(22 + i)) {
            id |= (1 << i);
        }
    }
    
    digitalWrite(TFT_RD, HIGH);
    digitalWrite(TFT_CS, HIGH);
    
    return id;
}

void identifyChip(uint16_t id) {
    Serial.print("\nChip Identification: ");
    
    switch (id) {
        case 0x9325:
            Serial.println("ILI9325 - Common old TFT driver");
            Serial.println("Use: TFT_ILI9325 library or MCUFRIEND_kbv");
            break;
        case 0x9328:
            Serial.println("ILI9328 - Similar to ILI9325");
            Serial.println("Use: MCUFRIEND_kbv");
            break;
        case 0x9341:
            Serial.println("ILI9341 - Very common modern TFT");
            Serial.println("Use: Adafruit_ILI9341 or MCUFRIEND_kbv");
            break;
        case 0x9342:
            Serial.println("ILI9342 - Similar to ILI9341");
            Serial.println("Use: MCUFRIEND_kbv");
            break;
        case 0x5408:
            Serial.println("HX8347 - Also common");
            Serial.println("Use: MCUFRIEND_kbv");
            break;
        case 0x7783:
            Serial.println("ST7783 - Common controller");
            Serial.println("Use: MCUFRIEND_kbv");
            break;
        case 0x8357:
            Serial.println("HX8357 - Large displays");
            Serial.println("Use: MCUFRIEND_kbv");
            break;
        case 0x0000:
            Serial.println("Display not responding or wrong pins!");
            Serial.println("Check wiring:");
            Serial.println("  - Port A (D22-D29) = Data pins D0-D7");
            Serial.println("  - A0 = RD (Read Enable)");
            Serial.println("  - A1 = WR (Write Enable)");
            Serial.println("  - A2 = RS (Register Select/DC)");
            Serial.println("  - A3 = CS (Chip Select)");
            Serial.println("  - A4 = RESET");
            break;
        case 0xFFFF:
            Serial.println("All pins high - display may be disconnected");
            break;
        default:
            Serial.print("Unknown chip ID: 0x");
            Serial.println(id, HEX);
            Serial.println("Might be: ILI9225, RM68140, or other variant");
    }
}

void testColors() {
    Serial.println("\nAttempting simple color write test...");
    
    // Set all data pins as outputs
    for (int i = 22; i <= 29; i++) {
        pinMode(i, OUTPUT);
    }
    
    // Try to write some data
    digitalWrite(TFT_CS, LOW);
    
    // Set address window
    writeCommand(0x2A);  // Column address
    writeData(0x0000);
    writeData(0x013F);   // 0-319
    
    writeCommand(0x2B);  // Row address
    writeData(0x0000);
    writeData(0x00EF);   // 0-239
    
    // Write memory
    writeCommand(0x2C);
    
    // Try writing RED (0xF800)
    for (int i = 0; i < 100; i++) {
        writeData(0xF800);  // Red
    }
    
    digitalWrite(TFT_CS, HIGH);
    
    Serial.println("Color write test complete. Check display for red pixels.");
}

void writeCommand(uint8_t cmd) {
    digitalWrite(TFT_RS, LOW);  // Command mode
    writeData(cmd);
}

void writeData(uint16_t data) {
    // High byte
    setDataBus((data >> 8) & 0xFF);
    digitalWrite(TFT_WR, LOW);
    delayMicroseconds(1);
    digitalWrite(TFT_WR, HIGH);
    delayMicroseconds(1);
    
    // Low byte
    setDataBus(data & 0xFF);
    digitalWrite(TFT_WR, LOW);
    delayMicroseconds(1);
    digitalWrite(TFT_WR, HIGH);
    delayMicroseconds(1);
    
    digitalWrite(TFT_RS, HIGH);  // Data mode
}

void setDataBus(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        digitalWrite(22 + i, (data >> i) & 1);
    }
}
