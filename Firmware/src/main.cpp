#include <Arduino.h>
#include <math.h>

#include "PZEM.h"
#include "Display.h"
#include "IoT.h"

// =====================================================
// OBJEK
// =====================================================

PZEM pzem;
Display display;
IoT iot;

// =====================================================
// INTERVAL
// =====================================================

unsigned long lastMeasurement = 0;

const unsigned long MEASUREMENT_INTERVAL = 2000;

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

    // Inisialisasi display lebih dahulu
    display.begin();

    // Inisialisasi pengukuran
    pzem.begin();

    // Inisialisasi IoT
    iot.begin();

    Serial.println("[SYSTEM] Initialization complete.");
}

// =====================================================
// LOOP
// =====================================================

void loop() {
    // Menangani koneksi Wi-Fi dan MQTT
    iot.loop();

    // Membaca PZEM setiap dua detik
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

            // Serial Monitor
            pzem.printData();

            // Update display
            display.showMeasurements(
                voltage,
                current,
                power,
                energy,
                frequency,
                pf
            );

            // Kirim data ke MQTT
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

            // Tampilkan nilai tidak tersedia
            display.showMeasurements(
                NAN, NAN, NAN, NAN, NAN, NAN
            );
        }
    }
}