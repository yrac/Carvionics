/**
 * SIMPLE DISPLAY TEST
 * 
 * Test if TFT display is working - draws simple colored squares
 * Upload this FIRST to verify display is operational
 * 
 * If display shows colors → display is working!
 * If display still white → check wiring
 */

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>

MCUFRIEND_kbv tft;

// Color definitions
#define BLACK       0x0000
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== SIMPLE DISPLAY TEST ===\n");
    
    // Read display ID
    uint16_t ID = tft.readID();
    Serial.print("Display ID: 0x");
    Serial.println(ID, HEX);
    
    // Initialize display
    Serial.println("Initializing display...");
    tft.begin(ID);
    tft.setRotation(0);
    
    Serial.println("Clearing screen...");
    tft.fillScreen(BLACK);
    delay(500);
    
    // Draw colored boxes
    Serial.println("Drawing test pattern...");
    
    // RED square (top-left)
    tft.fillRect(10, 10, 70, 70, RED);
    Serial.println("  RED drawn");
    delay(500);
    
    // GREEN square (top-right)
    tft.fillRect(240, 10, 70, 70, GREEN);
    Serial.println("  GREEN drawn");
    delay(500);
    
    // BLUE square (bottom-left)
    tft.fillRect(10, 160, 70, 70, BLUE);
    Serial.println("  BLUE drawn");
    delay(500);
    
    // YELLOW square (bottom-right)
    tft.fillRect(240, 160, 70, 70, YELLOW);
    Serial.println("  YELLOW drawn");
    delay(500);
    
    // Center CYAN circle
    tft.fillCircle(160, 120, 40, CYAN);
    Serial.println("  CYAN circle drawn");
    delay(500);
    
    // White border text area
    tft.drawRect(80, 200, 160, 30, WHITE);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(100, 210);
    tft.println("DISPLAY OK");
    Serial.println("  Text drawn");
    
    Serial.println("\n✓ Test complete!");
    Serial.println("If you see colored squares and text → Display works!");
    Serial.println("If screen is still white → Check wiring");
}

void loop() {
    delay(1000);
}
