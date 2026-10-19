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
// TARGET PF
// =====================================================

int targetPFValue = 90;

constexpr int TARGET_PF_MIN = 85;
constexpr int TARGET_PF_MAX = 95;

bool editingTargetPF = false;

// =====================================================
// INTERVAL PENGUKURAN
// =====================================================

unsigned long lastMeasurement = 0;
constexpr unsigned long MEASUREMENT_INTERVAL = 2000;

// =====================================================
// TARGET PF
// =====================================================

float getTargetPF() {
    return targetPFValue / 100.0f;
}

void updateTargetPFDisplay() {
    display.showTargetPF(
        getTargetPF(),
        editingTargetPF
    );
}

void handleEncoder() {
    // Selalu perbarui pembacaan GPIO terlebih dahulu.
    encoder.loop();

    // -------------------------------------------------
    // TOMBOL PUSH
    // -------------------------------------------------

    if (encoder.wasPressed()) {
        editingTargetPF = !editingTargetPF;

        if (editingTargetPF) {
            Serial.println("[TARGET PF] Edit mode ON");
        } else {
            Serial.print("[TARGET PF] Saved: ");
            Serial.println(getTargetPF(), 2);
        }

        updateTargetPFDisplay();
    }

    // -------------------------------------------------
    // ROTARY ENCODER
    // -------------------------------------------------

    int8_t rotation = encoder.getRotationDelta();

    if (!editingTargetPF || rotation == 0) {
        return;
    }

    targetPFValue += rotation;

    if (targetPFValue < TARGET_PF_MIN) {
        targetPFValue = TARGET_PF_MIN;
    }

    if (targetPFValue > TARGET_PF_MAX) {
        targetPFValue = TARGET_PF_MAX;
    }

    updateTargetPFDisplay();

    Serial.print("[TARGET PF] Value: ");
    Serial.println(getTargetPF(), 2);
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

    updateTargetPFDisplay();

    Serial.println("[SYSTEM] Initialization complete");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    // Input encoder diproses pada setiap iterasi.
    handleEncoder();

    // Pengelolaan Wi-Fi dan MQTT.
    iot.loop();

    // Pembacaan PZEM setiap dua detik.
    if (millis() - lastMeasurement >= MEASUREMENT_INTERVAL) {
        lastMeasurement = millis();

        pzem.readData();

        if (pzem.isDataValid()) {
            float voltage   = pzem.getVoltage();
            float current   = pzem.getCurrent();
            float power     = pzem.getPower();
            float energy    = pzem.getEnergy();
            float frequency = pzem.getFrequency();
            float pf        = pzem.getPF();

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