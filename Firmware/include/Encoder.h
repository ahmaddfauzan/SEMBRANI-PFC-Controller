#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
public:
    Encoder();

    void begin();
    void loop();

    int8_t getRotationDelta();
    bool wasPressed();

private:
    static constexpr uint8_t PIN_CLK = 32;
    static constexpr uint8_t PIN_DT  = 33;
    static constexpr uint8_t PIN_SW  = 25;

    static constexpr unsigned long DEBOUNCE_DELAY = 35;

    uint8_t lastEncoderState;
    int8_t encoderAccumulator;
    int8_t rotationDelta;

    bool lastButtonReading;
    bool stableButtonState;
    bool buttonPressedEvent;

    unsigned long lastDebounceTime;

    void readEncoder();
    void readButton();
};

#endif