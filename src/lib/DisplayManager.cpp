#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : initialized_(false) {
}

bool DisplayManager::begin() {
    uint16_t ID = tft_.readID();
    // Inisialisasi generic untuk berbagai ID panel
    tft_.begin(ID);
    tft_.setRotation(1);  // Landscape 320x240
    initialized_ = true;

    fillScreen(Color::BLACK);
    return true;
}

void DisplayManager::fillScreen(Color color) {
    if (!initialized_) return;
    tft_.fillScreen((uint16_t)color);
}

void DisplayManager::drawPixel(int16_t x, int16_t y, Color color) {
    if (!initialized_) return;
    tft_.drawPixel(x, y, (uint16_t)color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Color color) {
    if (!initialized_) return;
    tft_.drawLine(x0, y0, x1, y1, (uint16_t)color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
    if (!initialized_) return;
    tft_.drawRect(x, y, w, h, (uint16_t)color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, Color color) {
    if (!initialized_) return;
    tft_.fillRect(x, y, w, h, (uint16_t)color);
}

void DisplayManager::drawCircle(int16_t x, int16_t y, int16_t r, Color color) {
    if (!initialized_) return;
    tft_.drawCircle(x, y, r, (uint16_t)color);
}

void DisplayManager::fillCircle(int16_t x, int16_t y, int16_t r, Color color) {
    if (!initialized_) return;
    tft_.fillCircle(x, y, r, (uint16_t)color);
}

void DisplayManager::setTextColor(Color fg, Color bg) {
    if (!initialized_) return;
    tft_.setTextColor((uint16_t)fg, (uint16_t)bg);
}

void DisplayManager::setTextSize(uint8_t size) {
    if (!initialized_) return;
    tft_.setTextSize(size);
}

void DisplayManager::setFont(const GFXfont *font) {
    if (!initialized_) return;
    tft_.setFont(font);
}

void DisplayManager::setCursor(int16_t x, int16_t y) {
    if (!initialized_) return;
    tft_.setCursor(x, y);
}

void DisplayManager::print(const char *text) {
    if (!initialized_) return;
    tft_.print(text);
}

void DisplayManager::printf(const char *format, ...) {
    if (!initialized_) return;
    
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    tft_.print(buffer);
}

void DisplayManager::printCentered(int16_t y, const char *text, Color fg, Color bg, uint8_t size) {
    if (!initialized_) return;
    
    setTextColor(fg, bg);
    setTextSize(size);
    
    // Rough estimation: setiap character ~6 pixel width (default font)
    int16_t text_len = strlen(text);
    int16_t text_width = text_len * 6 * size;
    int16_t x = (SCREEN_WIDTH - text_width) / 2;
    
    setCursor(x, y);
    print(text);
}

void DisplayManager::debugPrintInfo() const {
    Serial.println("\n=== DisplayManager ===");
    Serial.print("Initialized: "); Serial.println(initialized_ ? "Yes" : "No");
    Serial.print("Resolution: "); Serial.print(SCREEN_WIDTH); Serial.print("x"); Serial.println(SCREEN_HEIGHT);
}

void DisplayManager::drawBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, Color border, Color fill) {
    fillRect(x, y, w, h, fill);
    drawRect(x, y, w, h, border);
}

void DisplayManager::drawParameterBox(uint16_t x, uint16_t y, const char *label, const char *value, Color fg) {
    setTextSize(1);
    setTextColor(fg, Color::DARK_GRAY);
    setCursor(x, y);
    print(label);
    setCursor(x, y + 10);
    print(value);
}
