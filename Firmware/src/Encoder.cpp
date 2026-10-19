#include "Encoder.h"

static const int8_t ENCODER_TRANSITIONS[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

Encoder::Encoder()
    : lastEncoderState(0),
      encoderAccumulator(0),
      rotationDelta(0),
      lastButtonReading(HIGH),
      stableButtonState(HIGH),
      buttonPressedEvent(false),
      lastDebounceTime(0) {
}

void Encoder::begin() {
    pinMode(PIN_CLK, INPUT_PULLUP);
    pinMode(PIN_DT, INPUT_PULLUP);
    pinMode(PIN_SW, INPUT_PULLUP);

    uint8_t clk = digitalRead(PIN_CLK);
    uint8_t dt  = digitalRead(PIN_DT);

    lastEncoderState = (clk << 1) | dt;

    lastButtonReading = digitalRead(PIN_SW);
    stableButtonState = lastButtonReading;

    lastDebounceTime = millis();

    Serial.println("[Encoder] Initialized");
    Serial.println("[Encoder] CLK=32, DT=33, SW=25");
}

void Encoder::loop() {
    readEncoder();
    readButton();
}

void Encoder::readEncoder() {
    uint8_t clk = digitalRead(PIN_CLK);
    uint8_t dt  = digitalRead(PIN_DT);

    uint8_t currentState = (clk << 1) | dt;

    if (currentState == lastEncoderState) {
        return;
    }

    uint8_t index = (lastEncoderState << 2) | currentState;

    encoderAccumulator += ENCODER_TRANSITIONS[index];
    lastEncoderState = currentState;

    // Satu langkah mekanis umumnya menghasilkan empat transisi.
    if (encoderAccumulator >= 4) {
        rotationDelta++;
        encoderAccumulator = 0;
    } else if (encoderAccumulator <= -4) {
        rotationDelta--;
        encoderAccumulator = 0;
    }
}

void Encoder::readButton() {
    const unsigned long now = millis();
    const bool reading = digitalRead(PIN_SW);

    if (reading != lastButtonReading) {
        lastButtonReading = reading;
        lastDebounceTime = now;
    }

    if (now - lastDebounceTime < DEBOUNCE_DELAY) {
        return;
    }

    if (reading != stableButtonState) {
        stableButtonState = reading;

        // INPUT_PULLUP: tombol ditekan ketika LOW.
        if (stableButtonState == LOW) {
            buttonPressedEvent = true;
            Serial.println("[Encoder] Button pressed");
        }
    }
}

int8_t Encoder::getRotationDelta() {
    int8_t result = rotationDelta;
    rotationDelta = 0;
    return result;
}

bool Encoder::wasPressed() {
    if (!buttonPressedEvent) {
        return false;
    }

    buttonPressedEvent = false;
    return true;
}