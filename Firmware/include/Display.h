#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum class DisplayMenu : uint8_t {
    MODE = 0,
    CAPACITOR_STEP,
    TARGET_PF,
    PAGE
};

struct DisplayConfig {
    bool threePhase;
    float capacitorPerStepKvar;
    float targetPF;
    uint8_t page;
    uint8_t pageCount;
};

class Display {
public:
    Display();

    void begin();

    void showMeasurements(
        float voltage,
        float current,
        float power,
        float energy,
        float frequency,
        float pf
    );

    void updateMenu(
        const DisplayConfig& config,
        DisplayMenu selectedMenu,
        bool frameVisible,
        bool editing
    );

    void showPageIndicator(uint8_t page, uint8_t pageCount);

private:
    TFT_eSPI tft;

    DisplayConfig currentConfig;

    DisplayMenu currentMenu;
    bool currentFrameVisible;
    bool currentEditing;

    float voltageValue;
    float currentValue;
    float powerValue;
    float energyValue;
    float frequencyValue;
    float pfValue;

    void drawLayout();
    void drawHeader();
    void drawMeasurements();
    void drawFooter();
    void drawMenuItem(
        int x,
        int width,
        DisplayMenu menu,
        const char* label,
        const char* value
    );

    void drawValue(
        int x,
        int y,
        float value,
        const char* unit,
        uint8_t decimals
    );
};

#endif