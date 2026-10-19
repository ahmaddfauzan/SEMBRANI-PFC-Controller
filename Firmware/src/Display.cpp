#include "Display.h"
#include <math.h>

#define BG_COLOR       TFT_BLACK
#define PANEL_COLOR    0x18E3
#define TEXT_COLOR     TFT_WHITE
#define LABEL_COLOR    0xBDF7
#define ACCENT_COLOR   TFT_CYAN
#define VALUE_COLOR   TFT_GREEN
#define ERROR_COLOR    TFT_RED

Display::Display() {
}

void Display::begin() {
    Serial.println("[Display] Initializing ST7789...");

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(BG_COLOR);

    drawLayout();

    Serial.println("[Display] Initialized.");
}

void Display::drawLayout() {
    tft.fillScreen(BG_COLOR);

    // Header
    tft.setTextSize(2);
    tft.setTextColor(ACCENT_COLOR, BG_COLOR);
    tft.setCursor(10, 8);
    tft.print("SEMBRANI PFC");

    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);
    tft.setCursor(10, 29);
    tft.print("POWER MONITORING SYSTEM");

    tft.drawFastHLine(10, 44, 300, ACCENT_COLOR);

    // Enam panel pengukuran
    drawMeasurement(10, 54, "VOLTAGE", NAN, "V", 1);
    drawMeasurement(165, 54, "CURRENT", NAN, "A", 3);

    drawMeasurement(10, 111, "ACTIVE POWER", NAN, "W", 1);
    drawMeasurement(165, 111, "ENERGY", NAN, "kWh", 3);

    drawMeasurement(10, 168, "FREQUENCY", NAN, "Hz", 1);
    drawMeasurement(165, 168, "POWER FACTOR", NAN, "PF", 2);

    // Footer
    tft.drawFastHLine(10, 225, 300, ACCENT_COLOR);

    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);
    tft.setCursor(10, 231);
    tft.print("SEMBRANI | ESP32 | PZEM-004T");
}

void Display::drawMeasurement(
    int x,
    int y,
    const char* label,
    float value,
    const char* unit,
    uint8_t decimals
) {
    const int width = 145;
    const int height = 48;

    tft.fillRoundRect(
        x, y, width, height, 5, PANEL_COLOR
    );

    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, PANEL_COLOR);
    tft.setCursor(x + 7, y + 5);
    tft.print(label);

    tft.setTextSize(2);
    tft.setCursor(x + 7, y + 21);

    if (isnan(value) || isinf(value)) {
        tft.setTextColor(ERROR_COLOR, PANEL_COLOR);
        tft.print("--");
    } else {
        tft.setTextColor(VALUE_COLOR, PANEL_COLOR);
        tft.print(value, decimals);
    }

    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, PANEL_COLOR);
    tft.setCursor(x + 105, y + 30);
    tft.print(unit);
}

void Display::showMeasurements(
    float voltage,
    float current,
    float power,
    float energy,
    float frequency,
    float pf
) {
    // Hanya memperbarui area nilai
    drawMeasurement(10, 54, "VOLTAGE", voltage, "V", 1);
    drawMeasurement(165, 54, "CURRENT", current, "A", 3);

    drawMeasurement(10, 111, "ACTIVE POWER", power, "W", 1);
    drawMeasurement(165, 111, "ENERGY", energy, "kWh", 3);

    drawMeasurement(10, 168, "FREQUENCY", frequency, "Hz", 1);
    drawMeasurement(165, 168, "POWER FACTOR", pf, "PF", 2);
}