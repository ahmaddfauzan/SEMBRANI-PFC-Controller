#include "Display.h"
#include <math.h>

// =====================================================
// WARNA
// =====================================================

#define BG_COLOR       TFT_BLACK
#define HEADER_COLOR   TFT_CYAN
#define LABEL_COLOR    TFT_WHITE
#define VALUE_COLOR    TFT_GREEN
#define ERROR_COLOR    TFT_RED
#define LINE_COLOR     0x4208

// =====================================================
// KONSTRUKTOR
// =====================================================

Display::Display() {
}

// =====================================================
// INISIALISASI
// =====================================================

void Display::begin() {
    Serial.println("[Display] Initializing ST7789...");

    tft.init();
    tft.setRotation(1);
    tft.invertDisplay(false);

    Serial.printf(
        "[Display] Width=%d Height=%d\n",
        tft.width(),
        tft.height()
    );

    tft.fillScreen(BG_COLOR);

    drawLayout();

    Serial.println("[Display] Initialization complete.");
}

// =====================================================
// LAYOUT
// =====================================================

void Display::drawLayout() {
    tft.fillScreen(BG_COLOR);

    // Header
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(HEADER_COLOR, BG_COLOR);
    tft.setCursor(10, 8);
    tft.print("SEMBRANI PFC");

    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);
    tft.setCursor(10, 30);
    tft.print("POWER MONITORING SYSTEM");

    tft.drawFastHLine(
        10, 45, tft.width() - 20, HEADER_COLOR
    );

    // Parameter pengukuran
    const int x1 = 10;
    const int x2 = 165;

    drawValue(x1, 54, NAN, "V", 1);
    drawValue(x2, 54, NAN, "A", 3);

    drawValue(x1, 111, NAN, "W", 1);
    drawValue(x2, 111, NAN, "kWh", 3);

    drawValue(x1, 168, NAN, "Hz", 1);
    drawValue(x2, 168, NAN, "PF", 2);

    // Label parameter
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);

    tft.setCursor(x1, 54);
    tft.print("VOLTAGE");

    tft.setCursor(x2, 54);
    tft.print("CURRENT");

    tft.setCursor(x1, 111);
    tft.print("ACTIVE POWER");

    tft.setCursor(x2, 111);
    tft.print("ENERGY");

    tft.setCursor(x1, 168);
    tft.print("FREQUENCY");

    tft.setCursor(x2, 168);
    tft.print("POWER FACTOR");

    // Footer
    tft.drawFastHLine(
        10, 225, tft.width() - 20, LINE_COLOR
    );

    tft.setTextColor(LABEL_COLOR, BG_COLOR);
    tft.setCursor(10, 231);
    tft.print("ESP32 | PZEM-004T");
}

// =====================================================
// GAMBAR NILAI
// =====================================================

void Display::drawValue(
    int x,
    int y,
    float value,
    const char* unit,
    uint8_t decimals
) {
    // Area khusus untuk nilai
    tft.fillRect(x, y + 17, 145, 28, BG_COLOR);

    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setCursor(x, y + 20);

    if (isnan(value) || isinf(value)) {
        tft.setTextColor(ERROR_COLOR, BG_COLOR);
        tft.print("--");
    } else {
        tft.setTextColor(VALUE_COLOR, BG_COLOR);
        tft.print(value, decimals);
    }

    // Satuan
    tft.setTextSize(1);
    tft.setTextColor(HEADER_COLOR, BG_COLOR);
    tft.setCursor(x + 108, y + 32);
    tft.print(unit);
}

// =====================================================
// UPDATE PENGUKURAN
// =====================================================

void Display::showMeasurements(
    float voltage,
    float current,
    float power,
    float energy,
    float frequency,
    float pf
) {
    drawValue(10, 54, voltage, "V", 1);
    drawValue(165, 54, current, "A", 3);

    drawValue(10, 111, power, "W", 1);
    drawValue(165, 111, energy, "kWh", 3);

    drawValue(10, 168, frequency, "Hz", 1);
    drawValue(165, 168, pf, "PF", 2);
}