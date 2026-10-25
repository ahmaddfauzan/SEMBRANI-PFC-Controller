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
#define SELECT_COLOR   TFT_YELLOW

// =====================================================
// KONSTRUKTOR
// =====================================================

Display::Display()
    : currentMenu(DisplayMenu::MODE),
      currentFrameVisible(false),
      currentEditing(false),
      voltageValue(NAN),
      currentValue(NAN),
      powerValue(NAN),
      energyValue(NAN),
      frequencyValue(NAN),
      pfValue(NAN) {

    currentConfig.threePhase = false;
    currentConfig.capacitorPerStepKvar = 5.0f;
    currentConfig.targetPF = 0.90f;
    currentConfig.page = 1;
    currentConfig.pageCount = 1;
}

// =====================================================
// INISIALISASI
// =====================================================

void Display::begin() {
    Serial.println("[Display] Initializing ST7789...");

    tft.init();
    tft.setRotation(1);
    tft.invertDisplay(false);
    tft.fillScreen(BG_COLOR);

    drawLayout();

    Serial.printf(
        "[Display] Width=%d Height=%d\n",
        tft.width(),
        tft.height()
    );

    Serial.println("[Display] Initialization complete.");
}

// =====================================================
// LAYOUT
// =====================================================

void Display::drawLayout() {
    tft.fillScreen(BG_COLOR);

    drawHeader();
    drawMeasurements();
    drawFooter();
}

// =====================================================
// HEADER EMPAT MENU
// =====================================================

void Display::drawHeader() {
    const int screenWidth = tft.width();

    // Judul kecil.
    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(HEADER_COLOR, BG_COLOR);
    tft.setCursor(8, 3);
    tft.print("SEMBRANI PFC");

    const int menuY = 17;
    const int menuH = 40;
    const int margin = 5;
    const int gap = 3;

    const int menuW =
        (screenWidth - 2 * margin - 3 * gap) / 4;

    char modeValue[8];
    char capacitorValue[16];
    char pfText[8];
    char pageValue[12];

    snprintf(
        modeValue,
        sizeof(modeValue),
        "%s",
        currentConfig.threePhase ? "3Ph" : "1Ph"
    );

    snprintf(
        capacitorValue,
        sizeof(capacitorValue),
        "%.1f kVAr",
        currentConfig.capacitorPerStepKvar
    );

    snprintf(
        pfText,
        sizeof(pfText),
        "%.2f",
        currentConfig.targetPF
    );

    snprintf(
        pageValue,
        sizeof(pageValue),
        "%u/%u",
        currentConfig.page,
        currentConfig.pageCount
    );

    drawMenuItem(
        margin,
        menuW,
        DisplayMenu::MODE,
        "MODE",
        modeValue
    );

    drawMenuItem(
        margin + (menuW + gap),
        menuW,
        DisplayMenu::CAPACITOR_STEP,
        "C/STEP",
        capacitorValue
    );

    drawMenuItem(
        margin + 2 * (menuW + gap),
        menuW,
        DisplayMenu::TARGET_PF,
        "TARGET PF",
        pfText
    );

    drawMenuItem(
        margin + 3 * (menuW + gap),
        menuW,
        DisplayMenu::PAGE,
        "PAGE",
        pageValue
    );

    tft.drawFastHLine(
        5,
        61,
        screenWidth - 10,
        HEADER_COLOR
    );
}

// =====================================================
// GAMBAR SATU MENU
// =====================================================

void Display::drawMenuItem(
    int x,
    int width,
    DisplayMenu menu,
    const char* label,
    const char* value
) {
    const int y = 17;
    const int height = 40;

    const bool selected =
        currentFrameVisible && currentMenu == menu;

    const bool editing =
        selected && currentEditing;

    const uint16_t bg =
        editing ? TFT_WHITE : BG_COLOR;

    const uint16_t fg =
        editing ? TFT_BLACK : LABEL_COLOR;

    tft.fillRect(x, y, width, height, bg);

    if (selected) {
        if (editing) {
            tft.drawRect(
                x,
                y,
                width,
                height,
                TFT_WHITE
            );
        } else {
            tft.drawRect(
                x,
                y,
                width,
                height,
                SELECT_COLOR
            );
        }
    }

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(fg, bg);

    tft.setCursor(x + 3, y + 5);
    tft.print(label);

    tft.setCursor(x + 3, y + 22);
    tft.print(value);
}

// =====================================================
// MONITORING PARAMETER
// =====================================================

void Display::drawMeasurements() {
    const int x1 = 10;
    const int x2 = 165;

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);

    tft.setCursor(x1, 69);
    tft.print("VOLTAGE");

    tft.setCursor(x2, 69);
    tft.print("CURRENT");

    drawValue(x1, 69, voltageValue, "V", 1);
    drawValue(x2, 69, currentValue, "A", 3);

    tft.setCursor(x1, 121);
    tft.print("ACTIVE POWER");

    tft.setCursor(x2, 121);
    tft.print("ENERGY");

    drawValue(x1, 121, powerValue, "W", 1);
    drawValue(x2, 121, energyValue, "kWh", 3);

    tft.setCursor(x1, 173);
    tft.print("FREQUENCY");

    tft.setCursor(x2, 173);
    tft.print("POWER FACTOR");

    drawValue(x1, 173, frequencyValue, "Hz", 1);
    drawValue(x2, 173, pfValue, "PF", 2);
}

// =====================================================
// UPDATE DATA PENGUKURAN
// =====================================================

void Display::showMeasurements(
    float voltage,
    float current,
    float power,
    float energy,
    float frequency,
    float pf
) {
    voltageValue = voltage;
    currentValue = current;
    powerValue = power;
    energyValue = energy;
    frequencyValue = frequency;
    pfValue = pf;

    drawMeasurements();
}

// =====================================================
// NILAI DAN SATUAN
// =====================================================

void Display::drawValue(
    int x,
    int y,
    float value,
    const char* unit,
    uint8_t decimals
) {
    // Hapus area nilai saja.
    tft.fillRect(x, y + 14, 145, 25, BG_COLOR);

    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setCursor(x, y + 16);

    if (isnan(value) || isinf(value)) {
        tft.setTextColor(ERROR_COLOR, BG_COLOR);
        tft.print("--");
    } else {
        tft.setTextColor(VALUE_COLOR, BG_COLOR);
        tft.print(value, decimals);
    }

    tft.setTextSize(1);
    tft.setTextColor(HEADER_COLOR, BG_COLOR);
    tft.setCursor(x + 108, y + 29);
    tft.print(unit);
}

// =====================================================
// UPDATE MENU
// =====================================================

void Display::updateMenu(
    const DisplayConfig& config,
    DisplayMenu selectedMenu,
    bool frameVisible,
    bool editing
) {
    currentConfig = config;
    currentMenu = selectedMenu;
    currentFrameVisible = frameVisible;
    currentEditing = editing;

    drawHeader();
}

// =====================================================
// INDIKATOR HALAMAN
// =====================================================

void Display::showPageIndicator(
    uint8_t page,
    uint8_t pageCount
) {
    currentConfig.page = page;
    currentConfig.pageCount = pageCount;

    drawHeader();
    drawFooter();
}

// =====================================================
// FOOTER
// =====================================================

void Display::drawFooter() {
    const int y = 224;

    tft.fillRect(
        0,
        y,
        tft.width(),
        tft.height() - y,
        BG_COLOR
    );

    tft.drawFastHLine(
        5,
        y,
        tft.width() - 10,
        LINE_COLOR
    );

    tft.setTextFont(1);
    tft.setTextSize(1);
    tft.setTextColor(LABEL_COLOR, BG_COLOR);

    tft.setCursor(8, 231);
    tft.print(
        currentConfig.threePhase
            ? "PM1200 | RS485"
            : "ESP32 | PZEM-004T"
    );

    char pageText[20];

    snprintf(
        pageText,
        sizeof(pageText),
        "PAGE %u/%u",
        currentConfig.page,
        currentConfig.pageCount
    );

    const int textWidth = tft.textWidth(pageText);

    tft.setCursor(
        tft.width() - textWidth - 8,
        231
    );

    tft.print(pageText);
}