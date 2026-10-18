#include <Arduino.h>
#include "PZEM.h"
#include "Display.h"

// ========================================
// INISIALISASI OBJEK
// ========================================

PZEM pzem;
Display display;

// ========================================
// SETUP
// ========================================

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("================================");
    Serial.println("    SEMBRANI POWER MONITOR");
    Serial.println("================================");

    // Inisialisasi PZEM
    pzem.begin();

    // Inisialisasi TFT LCD
    display.begin();

    Serial.println("System initialized.");
}

// ========================================
// LOOP
// ========================================

void loop()
{
    // Membaca parameter listrik
    pzem.readData();

    // Memeriksa validitas data
    if (pzem.isDataValid())
    {
        // Menampilkan data ke Serial Monitor
        pzem.printData();

        // Menampilkan data ke TFT LCD
        display.showMeasurements(
            pzem.getVoltage(),
            pzem.getCurrent(),
            pzem.getPower(),
            pzem.getEnergy(),
            pzem.getFrequency(),
            pzem.getPF()
        );
    }
    else
    {
        Serial.println("PZEM communication error.");

        // Kirim nilai invalid agar TFT menampilkan pesan error
        display.showMeasurements(
            NAN, NAN, NAN, NAN, NAN, NAN
        );
    }

    delay(2000);
}