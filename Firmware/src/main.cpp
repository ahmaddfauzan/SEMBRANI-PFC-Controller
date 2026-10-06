#include <Arduino.h>

#include "PowerMeter.h"
#include "IoT.h"

// =====================================================
// WiFi Configuration
// =====================================================

const char* WIFI_SSID = "R PRO JOGJA";
const char* WIFI_PASSWORD = "wifigratis21";

// =====================================================
// MQTT Configuration
// =====================================================

const char* MQTT_SERVER = "broker.emqx.io";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "sembrani/powermeter";

// =====================================================
// System Modules
// =====================================================

PowerMeter powerMeter(
    Serial2,
    16,     // RX
    17,     // TX
    4,      // DE + RE control
    1       // PM1200 Slave ID
);

IoT iot(
    WIFI_SSID,
    WIFI_PASSWORD,
    MQTT_SERVER,
    MQTT_PORT,
    MQTT_TOPIC
);

// =====================================================
// Setup
// =====================================================

void setup()
{
    Serial.begin(9600);
    delay(1000);

    Serial.println();
    Serial.println("====================================");
    Serial.println("     SEMBRANI PFC CONTROLLER");
    Serial.println("====================================");

    // Initialize Power Meter
    Serial.println("Initializing PowerMeter...");
    powerMeter.begin(9600);
    Serial.println("PowerMeter initialized");

    // Initialize IoT
    Serial.println("Initializing IoT...");
    iot.begin();
    Serial.println("IoT initialized");

    Serial.println("====================================");
    Serial.println("System ready");
    Serial.println("====================================");
}

// =====================================================
// Main Loop
// =====================================================

void loop()
{
    // Maintain WiFi and MQTT connection
    iot.loop();

    // Read Power Meter
    PowerData data;

    if (powerMeter.read(data))
    {
        // =================================================
        // TOTAL
        // =================================================

        Serial.println();
        Serial.println("----------- POWER DATA -----------");

        Serial.print("Active Power    : ");
        Serial.print(data.activePower);
        Serial.println(" W");

        Serial.print("Reactive Power  : ");
        Serial.print(data.reactivePower);
        Serial.println(" VAR");

        Serial.print("Apparent Power  : ");
        Serial.print(data.apparentPower);
        Serial.println(" VA");

        Serial.print("Power Factor    : ");
        Serial.println(data.powerFactor, 3);

        Serial.print("Voltage L-L     : ");
        Serial.print(data.voltageLL);
        Serial.println(" V");

        Serial.print("Voltage L-N     : ");
        Serial.print(data.voltageLN);
        Serial.println(" V");

        Serial.print("Current         : ");
        Serial.print(data.current);
        Serial.println(" A");

        Serial.print("Frequency       : ");
        Serial.print(data.frequency);
        Serial.println(" Hz");

        Serial.print("Interruptions   : ");
        Serial.println(data.interruptionCount);

        // =================================================
        // VOLTAGE
        // =================================================

        Serial.println();
        Serial.println("----------- VOLTAGE -----------");

        Serial.print("L1-N            : ");
        Serial.print(data.voltageL1N);
        Serial.println(" V");

        Serial.print("L2-N            : ");
        Serial.print(data.voltageL2N);
        Serial.println(" V");

        Serial.print("L3-N            : ");
        Serial.print(data.voltageL3N);
        Serial.println(" V");

        Serial.print("L1-L2           : ");
        Serial.print(data.voltageL1L2);
        Serial.println(" V");

        Serial.print("L2-L3           : ");
        Serial.print(data.voltageL2L3);
        Serial.println(" V");

        Serial.print("L1-L3           : ");
        Serial.print(data.voltageL1L3);
        Serial.println(" V");

        // =================================================
        // PHASE 1
        // =================================================

        Serial.println();
        Serial.println("----------- PHASE 1 -----------");

        Serial.print("VA1             : ");
        Serial.print(data.apparentPowerL1);
        Serial.println(" VA");

        Serial.print("W1              : ");
        Serial.print(data.activePowerL1);
        Serial.println(" W");

        Serial.print("VAR1            : ");
        Serial.print(data.reactivePowerL1);
        Serial.println(" VAR");

        Serial.print("PF1             : ");
        Serial.println(data.powerFactorL1, 3);

        Serial.print("A1              : ");
        Serial.print(data.currentL1);
        Serial.println(" A");

        Serial.print("F1              : ");
        Serial.print(data.frequencyL1);
        Serial.println(" Hz");

        Serial.print("Interruptions 1 : ");
        Serial.println(data.interruptionL1);

        // =================================================
        // PHASE 2
        // =================================================

        Serial.println();
        Serial.println("----------- PHASE 2 -----------");

        Serial.print("VA2             : ");
        Serial.print(data.apparentPowerL2);
        Serial.println(" VA");

        Serial.print("W2              : ");
        Serial.print(data.activePowerL2);
        Serial.println(" W");

        Serial.print("VAR2            : ");
        Serial.print(data.reactivePowerL2);
        Serial.println(" VAR");

        Serial.print("PF2             : ");
        Serial.println(data.powerFactorL2, 3);

        Serial.print("A2              : ");
        Serial.print(data.currentL2);
        Serial.println(" A");

        Serial.print("F2              : ");
        Serial.print(data.frequencyL2);
        Serial.println(" Hz");

        Serial.print("Interruptions 2 : ");
        Serial.println(data.interruptionL2);

        // =================================================
        // PHASE 3
        // =================================================

        Serial.println();
        Serial.println("----------- PHASE 3 -----------");

        Serial.print("VA3             : ");
        Serial.print(data.apparentPowerL3);
        Serial.println(" VA");

        Serial.print("W3              : ");
        Serial.print(data.activePowerL3);
        Serial.println(" W");

        Serial.print("VAR3            : ");
        Serial.print(data.reactivePowerL3);
        Serial.println(" VAR");

        Serial.print("PF3             : ");
        Serial.println(data.powerFactorL3, 3);

        Serial.print("A3              : ");
        Serial.print(data.currentL3);
        Serial.println(" A");

        Serial.print("F3              : ");
        Serial.print(data.frequencyL3);
        Serial.println(" Hz");

        Serial.print("Interruptions 3 : ");
        Serial.println(data.interruptionL3);

        // =================================================
        // MQTT
        // =================================================

        if (iot.publishPowerData(data))
        {
            Serial.println();
            Serial.println("MQTT: Power data published");
        }
        else
        {
            Serial.println();
            Serial.println("MQTT: Publish failed");
        }

        Serial.println("--------------------------------");
    }
    else
    {
        Serial.print("PowerMeter read failed. Error: ");
        Serial.println(powerMeter.getLastError());
    }

    // Sampling interval
    delay(1000);
}