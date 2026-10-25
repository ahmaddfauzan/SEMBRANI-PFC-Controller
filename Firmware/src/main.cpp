#include <Arduino.h>
#include <math.h>

#include "PZEM.h"
#include "Display.h"
#include "IoT.h"
#include "Encoder.h"

// =====================================================
// OBJEK
// =====================================================

PZEM pzem;
Display display;
IoT iot;
Encoder encoder;

// =====================================================
// KONFIGURASI
// =====================================================

bool threePhaseMode = false;

float capacitorPerStepKvar = 5.0f;

int targetPFValue = 90;

constexpr int TARGET_PF_MIN = 85;
constexpr int TARGET_PF_MAX = 99;

constexpr float CAP_STEP_MIN = 0.1f;
constexpr float CAP_STEP_MAX = 100.0f;

uint8_t currentPage = 1;

DisplayMenu selectedMenu = DisplayMenu::MODE;

enum class UIState : uint8_t {
    NORMAL,
    NAVIGATING,
    EDITING
};

UIState uiState = UIState::NORMAL;

constexpr unsigned long UI_TIMEOUT = 4000;
constexpr unsigned long MEASUREMENT_INTERVAL = 2000;

unsigned long lastInteractionTime = 0;
unsigned long lastMeasurement = 0;

// =====================================================
// KONFIGURASI DISPLAY
// =====================================================

float getTargetPF() {
    return targetPFValue / 100.0f;
}

uint8_t getPageCount() {
    return threePhaseMode ? 4 : 1;
}

DisplayConfig getDisplayConfig() {
    DisplayConfig config;

    config.threePhase = threePhaseMode;
    config.capacitorPerStepKvar = capacitorPerStepKvar;
    config.targetPF = getTargetPF();
    config.page = currentPage;
    config.pageCount = getPageCount();

    return config;
}

void updateDisplayMenu() {
    display.updateMenu(
        getDisplayConfig(),
        selectedMenu,
        uiState != UIState::NORMAL,
        uiState == UIState::EDITING
    );
}

void registerInteraction() {
    lastInteractionTime = millis();
}

// =====================================================
// EDIT MENU
// =====================================================

void editSelectedMenu(int direction) {
    switch (selectedMenu) {
        case DisplayMenu::MODE:
            threePhaseMode = !threePhaseMode;
            currentPage = 1;
            break;

        case DisplayMenu::CAPACITOR_STEP:
            capacitorPerStepKvar += direction * 0.5f;

            if (capacitorPerStepKvar < CAP_STEP_MIN) {
                capacitorPerStepKvar = CAP_STEP_MIN;
            }

            if (capacitorPerStepKvar > CAP_STEP_MAX) {
                capacitorPerStepKvar = CAP_STEP_MAX;
            }
            break;

        case DisplayMenu::TARGET_PF:
            targetPFValue += direction;

            if (targetPFValue < TARGET_PF_MIN) {
                targetPFValue = TARGET_PF_MIN;
            }

            if (targetPFValue > TARGET_PF_MAX) {
                targetPFValue = TARGET_PF_MAX;
            }
            break;

        case DisplayMenu::PAGE: {
            const int pageCount = getPageCount();

            currentPage += direction;

            if (currentPage < 1) {
                currentPage = pageCount;
            }

            if (currentPage > pageCount) {
                currentPage = 1;
            }
            break;
        }
    }

    updateDisplayMenu();

    Serial.printf(
        "[MENU] Mode=%s C/Step=%.1f kVAr TargetPF=%.2f Page=%u/%u\n",
        threePhaseMode ? "3Ph" : "1Ph",
        capacitorPerStepKvar,
        getTargetPF(),
        currentPage,
        getPageCount()
    );
}

// =====================================================
// ENCODER
// =====================================================

void handleEncoder() {
    encoder.loop();

    const int8_t rotation = encoder.getRotationDelta();
    const bool pressed = encoder.wasPressed();

    if (rotation != 0) {
        registerInteraction();

        if (uiState == UIState::NORMAL) {
            uiState = UIState::NAVIGATING;
            selectedMenu = DisplayMenu::MODE;
        } else if (uiState == UIState::NAVIGATING) {
            int menuIndex = static_cast<int>(selectedMenu);
            menuIndex += rotation;

            if (menuIndex < 0) {
                menuIndex = 3;
            }

            if (menuIndex > 3) {
                menuIndex = 0;
            }

            selectedMenu = static_cast<DisplayMenu>(menuIndex);
        } else {
            editSelectedMenu(rotation);
        }

        updateDisplayMenu();
    }

    if (pressed) {
        registerInteraction();

        if (uiState == UIState::NAVIGATING) {
            uiState = UIState::EDITING;
        } else if (uiState == UIState::EDITING) {
            uiState = UIState::NAVIGATING;

            Serial.println("[MENU] Value confirmed.");
        }

        updateDisplayMenu();
    }

    if (uiState == UIState::NAVIGATING &&
        millis() - lastInteractionTime >= UI_TIMEOUT) {

        uiState = UIState::NORMAL;
        updateDisplayMenu();
    }
}

// =====================================================
// SETUP
// =====================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" SEMBRANI PFC CONTROLLER");
    Serial.println("================================");

    display.begin();
    encoder.begin();
    pzem.begin();
    iot.begin();

    updateDisplayMenu();

    Serial.println("[SYSTEM] Initialization complete.");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    handleEncoder();

    iot.loop();

    if (millis() - lastMeasurement >= MEASUREMENT_INTERVAL) {
        lastMeasurement = millis();

        // Tahap ini baru membaca PZEM.
        // Mode 3Ph akan dihubungkan ke PM1200 pada tahap berikutnya.
        if (!threePhaseMode) {
            pzem.readData();

            if (pzem.isDataValid()) {
                const float voltage = pzem.getVoltage();
                const float current = pzem.getCurrent();
                const float power = pzem.getPower();
                const float energy = pzem.getEnergy();
                const float frequency = pzem.getFrequency();
                const float pf = pzem.getPF();

                pzem.printData();

                display.showMeasurements(
                    voltage,
                    current,
                    power,
                    energy,
                    frequency,
                    pf
                );

                if (iot.isConnected()) {
                    iot.publishPowerData(
                        voltage,
                        current,
                        power,
                        energy,
                        frequency,
                        pf
                    );
                }
            } else {
                Serial.println("[PZEM] Communication error.");

                display.showMeasurements(
                    NAN, NAN, NAN, NAN, NAN, NAN
                );
            }
        }
    }
}