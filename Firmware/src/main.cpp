#include <Arduino.h>

#include "PZEM.h"
#include "Display.h"
#include "IoT.h"

PZEM pzem;
Display display;
IoT iot;

unsigned long lastMeasurement = 0;

const unsigned long MEASUREMENT_INTERVAL = 2000;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println(" SEMBRANI PFC CONTROLLER");
    Serial.println("================================");

    pzem.begin();
    display.begin();
    iot.begin();

    Serial.println("System initialized.");
}

void loop() {
    // Menangani koneksi Wi-Fi dan MQTT
    iot.loop();

    // Pembacaan pengukuran setiap 2 detik
    if (millis() - lastMeasurement >= MEASUREMENT_INTERVAL) {
        lastMeasurement = millis();

        pzem.readData();

        if (pzem.isDataValid()) {
            pzem.printData();

            float voltage   = pzem.getVoltage();
            float current   = pzem.getCurrent();
            float power     = pzem.getPower();
            float energy    = pzem.getEnergy();
            float frequency = pzem.getFrequency();
            float pf        = pzem.getPF();

            // Perbarui TFT LCD
            display.showMeasurements(
                voltage,
                current,
                power,
                energy,
                frequency,
                pf
            );

            // Kirim data ke dashboard melalui MQTT
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