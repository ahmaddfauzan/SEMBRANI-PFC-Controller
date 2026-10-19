#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>

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

private:
    TFT_eSPI tft;

    void drawLayout();

    void drawValue(
        int x,
        int y,
        float value,
        const char* unit,
        uint8_t decimals
    );
};

#endif