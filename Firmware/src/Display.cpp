#include "Display.h"

// ========================================
// KONFIGURASI TAMPILAN
// ========================================

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Warna RGB565
#define COLOR_BACKGROUND  0x0841
#define COLOR_PANEL       0x10A2
#define COLOR_WHITE       0xFFFF
#define COLOR_CYAN        0x07FF
#define COLOR_GREEN       0x07E0
#define COLOR_YELLOW      0xFFE0
#define COLOR_GRAY        0xBDF7
#define COLOR_RED         0xF800

// ========================================
// CONSTRUCTOR
// ========================================

Display::Display()
{
}

// ========================================
// INISIALISASI DISPLAY
// ========================================

void Display::begin()
{
    tft.init();

    // Orientasi portrait
    tft.setRotation(0);

    tft.fillScreen(COLOR_BACKGROUND);

    drawLayout();
}

// ========================================
// MEMBUAT LAYOUT
// ========================================

void Display::drawLayout()
{
    // Header
    tft.fillRect(
        0, 0,
        SCREEN_WIDTH, 48,
        COLOR_PANEL
    );

    tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
    tft.setTextSize(2);
    tft.setCursor(12, 8);
    tft.println("SEMBRANI");

    tft.setTextColor(COLOR_WHITE, COLOR_PANEL);
    tft.setTextSize(1);
    tft.setCursor(13, 31);
    tft.println("POWER MONITOR");

    // Garis pemisah header
    tft.drawFastHLine(
        0, 48,
        SCREEN_WIDTH,
        COLOR_CYAN
    );

    // Area pengukuran
    const int rowY[] = {
        58, 112, 166, 220, 274
    };

    for (int i = 0; i < 5; i++)
    {
        tft.fillRoundRect(
            8,
            rowY[i],
            224,
            46,
            5,
            COLOR_PANEL
        );
    }

    // Label pengukuran
    tft.setTextSize(1);
    tft.setTextColor(COLOR_GRAY, COLOR_PANEL);

    tft.setCursor(16, 65);
    tft.print("VOLTAGE");

    tft.setCursor(16, 119);
    tft.print("CURRENT");

    tft.setCursor(16, 173);
    tft.print("ACTIVE POWER");

    tft.setCursor(16, 227);
    tft.print("ENERGY");

    tft.setCursor(16, 281);
    tft.print("FREQUENCY");

    // Footer untuk power factor
    tft.fillRect(
        0, 304,
        SCREEN_WIDTH, 16,
        COLOR_BACKGROUND
    );

    tft.setTextColor(COLOR_YELLOW, COLOR_BACKGROUND);
    tft.setTextSize(1);
    tft.setCursor(12, 307);
    tft.print("POWER FACTOR:");

    tft.setTextColor(COLOR_WHITE, COLOR_BACKGROUND);
    tft.setCursor(112, 307);
    tft.print("--");
}

// ========================================
// MEMPERBARUI HASIL PENGUKURAN
// ========================================

void Display::showMeasurements(
    float voltage,
    float current,
    float power,
    float energy,
    float frequency,
    float pf
)
{
    // ====================================
    // VALIDASI DATA
    // ====================================

    if (
        isnan(voltage) ||
        isnan(current) ||
        isnan(power) ||
        isnan(energy) ||
        isnan(frequency) ||
        isnan(pf)
    )
    {
        tft.fillRect(
            0, 49,
            SCREEN_WIDTH, 255,
            COLOR_BACKGROUND
        );

        tft.setTextColor(COLOR_RED, COLOR_BACKGROUND);
        tft.setTextSize(2);
        tft.setCursor(15, 135);
        tft.println("PZEM ERROR");

        tft.setTextColor(COLOR_WHITE, COLOR_BACKGROUND);
        tft.setTextSize(1);
        tft.setCursor(15, 160);
        tft.println("Check communication");

        return;
    }

    // ====================================
    // VOLTAGE
    // ====================================

    tft.fillRect(14, 79, 212, 21, COLOR_PANEL);

    tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
    tft.setTextSize(2);
    tft.setCursor(16, 81);

    tft.print(voltage, 1);
    tft.print(" V");

    // ====================================
    // CURRENT
    // ====================================

    tft.fillRect(14, 133, 212, 21, COLOR_PANEL);

    tft.setTextColor(COLOR_GREEN, COLOR_PANEL);
    tft.setTextSize(2);
    tft.setCursor(16, 135);

    tft.print(current, 3);
    tft.print(" A");

    // ====================================
    // ACTIVE POWER
    // ====================================

    tft.fillRect(14, 187, 212, 21, COLOR_PANEL);

    tft.setTextColor(COLOR_YELLOW, COLOR_PANEL);
    tft.setTextSize(2);
    tft.setCursor(16, 189);

    tft.print(power, 1);
    tft.print(" W");

    // ====================================
    // ENERGY
    // ====================================

    tft.fillRect(14, 241, 212, 21, COLOR_PANEL);

    tft.setTextColor(COLOR_WHITE, COLOR_PANEL);
    tft.setTextSize(2);
    tft.setCursor(16, 243);

    tft.print(energy, 3);
    tft.print(" kWh");

    // ====================================
    // FREQUENCY
    // ====================================

    tft.fillRect(14, 295, 212, 7, COLOR_PANEL);

    tft.setTextColor(COLOR_CYAN, COLOR_PANEL);
    tft.setTextSize(1);
    tft.setCursor(16, 295);

    tft.print(frequency, 1);
    tft.print(" Hz");

    // ====================================
    // POWER FACTOR
    // ====================================

    tft.fillRect(112, 307, 116, 12, COLOR_BACKGROUND);

    tft.setTextColor(COLOR_GREEN, COLOR_BACKGROUND);
    tft.setTextSize(1);
    tft.setCursor(112, 307);

    tft.print(pf, 2);
}