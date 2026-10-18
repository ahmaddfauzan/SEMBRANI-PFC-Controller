#include <Arduino.h>
#include "PZEM.h"

// ========================================
// OBJEK PZEM
// ========================================

PZEM pzem;

// ========================================
// SETUP
// ========================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("      PZEM-004T ESP32");
    Serial.println("================================");

    pzem.begin();
}

// ========================================
// LOOP
// ========================================

void loop()
{
    pzem.readData();
    pzem.printData();

    delay(2000);
}