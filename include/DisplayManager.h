#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <stdint.h>
#include <Adafruit_GFX.h>

// Include MCUFRIEND_kbv library for 8-bit parallel TFT
#include <MCUFRIEND_kbv.h>

// FreeFonts untuk text yang lebih smooth (hanya untuk Mega 2560)
#if defined(__AVR_ATmega2560__)
    #define USE_FREEFONT 1
    #include <Fonts/FreeSansBold18pt7b.h>
    #include <Fonts/FreeSansBold12pt7b.h>
    #include <Fonts/FreeSans9pt7b.h>
#else
    #define USE_FREEFONT 0
#endif

/**
 * @class DisplayManager
 * @brief Mengelola inisialisasi TFT dan fungsi drawing primitif
 * 
 * Wrapper untuk MCUFRIEND_kbv + Adafruit_GFX
 * Handles:
 * - TFT initialization (8-bit parallel mode)
 * - Color definitions (EFIS style)
 * - Primitive drawing (text, rect, circle)
 * - Screen rotation & orientation
 */
class DisplayManager {
public:
    // Color palette (EFIS cockpit style)
    enum class Color : uint16_t {
        BLACK       = 0x0000,
        WHITE       = 0xFFFF,
        GREEN       = 0x07E0,   // Normal/OK
        DARK_GREEN  = 0x0380,   // Dimmed green
        AMBER       = 0xFDA0,   // Caution warning
        RED         = 0xF800,   // Critical warning
        CYAN        = 0x07FF,
        BLUE        = 0x001F,
        DARK_GRAY   = 0x4208,
        LIGHT_GRAY  = 0xC618
    };
    
    // Resolution constants
    static constexpr uint16_t SCREEN_WIDTH = 320;
    static constexpr uint16_t SCREEN_HEIGHT = 240;
    
    DisplayManager();
    
    // Initialize TFT display (must call before any drawing)
    bool begin();
    
    // Screen control
    void fillScreen(Color color);
    void clear() { fillScreen(Color::BLACK); }
    
    // Primitive drawing functions
    void drawPixel(int16_t x, int16_t y, Color color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Color color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color);
    void drawCircle(int16_t x, int16_t y, int16_t r, Color color);
    void fillCircle(int16_t x, int16_t y, int16_t r, Color color);
    
    // Text drawing
    void setTextColor(Color fg, Color bg = Color::BLACK);
    void setTextSize(uint8_t size);
    void setFont(const GFXfont *font = nullptr); // nullptr = built-in font
    void setCursor(int16_t x, int16_t y);
    void print(const char *text);
    void printf(const char *format, ...);
    
    // Centered text helpers
    void printCentered(int16_t y, const char *text, Color fg, Color bg = Color::BLACK, uint8_t size = 1);
    
    // Helper functions untuk UI rendering
    void drawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color border, Color fill);
    void drawParameterBox(uint16_t x, uint16_t y, const char *label, const char *value, Color fg);

    // Simple startup diagnostic: draw color bars to verify TFT
    void drawTestPattern();
    
    // Get display object for custom drawing
    MCUFRIEND_kbv* getTFT() { return &tft_; }
    
    // Debug
    void debugPrintInfo() const;

private:
    MCUFRIEND_kbv tft_;
    bool initialized_;
};

#endif
