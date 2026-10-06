#include "IoT.h"

IoT::IoT(
    const char* ssid,
    const char* password,
    const char* mqttServer,
    uint16_t mqttPort,
    const char* mqttTopic
)
    : _ssid(ssid),
      _password(password),
      _mqttServer(mqttServer),
      _mqttPort(mqttPort),
      _mqttTopic(mqttTopic),
      _mqtt(_wifiClient)
{
}

void IoT::begin()
{
    connectWiFi();

    _mqtt.setServer(_mqttServer, _mqttPort);

    // Buffer MQTT diperbesar karena JSON PowerData cukup besar
    _mqtt.setBufferSize(1024);

    connectMQTT();
}

void IoT::loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }

    if (!_mqtt.connected()) {
        connectMQTT();
    }

    _mqtt.loop();
}

void IoT::connectWiFi()
{
    Serial.println("Connecting to WiFi...");

    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void IoT::connectMQTT()
{
    Serial.println("Connecting to MQTT...");

    while (!_mqtt.connected()) {

        String clientId = "SEMBRANI-";
        clientId += String(random(0xffff), HEX);

        if (_mqtt.connect(clientId.c_str())) {
            Serial.println("MQTT connected");
        }
        else {
            Serial.print("MQTT failed, state = ");
            Serial.println(_mqtt.state());

            delay(2000);
        }
    }
}

bool IoT::publishPowerData(const PowerData& data)
{
    if (!_mqtt.connected()) {
        return false;
    }

    String json = powerDataToJson(data);

    return _mqtt.publish(_mqttTopic, json.c_str());
}

bool IoT::isConnected()
{
    return _mqtt.connected();
}

String IoT::powerDataToJson(const PowerData& data)
{
    String json = "{";

    // Total / system
    json += "\"apparentPower\":" + String(data.apparentPower, 2) + ",";
    json += "\"activePower\":" + String(data.activePower, 2) + ",";
    json += "\"reactivePower\":" + String(data.reactivePower, 2) + ",";
    json += "\"powerFactor\":" + String(data.powerFactor, 3) + ",";
    json += "\"voltageLL\":" + String(data.voltageLL, 2) + ",";
    json += "\"voltageLN\":" + String(data.voltageLN, 2) + ",";
    json += "\"current\":" + String(data.current, 2) + ",";
    json += "\"frequency\":" + String(data.frequency, 2) + ",";
    json += "\"interruptionCount\":" + String(data.interruptionCount) + ",";

    // Voltage
    json += "\"voltageL1N\":" + String(data.voltageL1N, 2) + ",";
    json += "\"voltageL2N\":" + String(data.voltageL2N, 2) + ",";
    json += "\"voltageL3N\":" + String(data.voltageL3N, 2) + ",";
    json += "\"voltageL1L2\":" + String(data.voltageL1L2, 2) + ",";
    json += "\"voltageL2L3\":" + String(data.voltageL2L3, 2) + ",";
    json += "\"voltageL1L3\":" + String(data.voltageL1L3, 2) + ",";

    // Phase 1
    json += "\"apparentPowerL1\":" + String(data.apparentPowerL1, 2) + ",";
    json += "\"activePowerL1\":" + String(data.activePowerL1, 2) + ",";
    json += "\"reactivePowerL1\":" + String(data.reactivePowerL1, 2) + ",";
    json += "\"powerFactorL1\":" + String(data.powerFactorL1, 3) + ",";
    json += "\"currentL1\":" + String(data.currentL1, 2) + ",";
    json += "\"frequencyL1\":" + String(data.frequencyL1, 2) + ",";
    json += "\"interruptionL1\":" + String(data.interruptionL1) + ",";

    // Phase 2
    json += "\"apparentPowerL2\":" + String(data.apparentPowerL2, 2) + ",";
    json += "\"activePowerL2\":" + String(data.activePowerL2, 2) + ",";
    json += "\"reactivePowerL2\":" + String(data.reactivePowerL2, 2) + ",";
    json += "\"powerFactorL2\":" + String(data.powerFactorL2, 3) + ",";
    json += "\"currentL2\":" + String(data.currentL2, 2) + ",";
    json += "\"frequencyL2\":" + String(data.frequencyL2, 2) + ",";
    json += "\"interruptionL2\":" + String(data.interruptionL2) + ",";

    // Phase 3
    json += "\"apparentPowerL3\":" + String(data.apparentPowerL3, 2) + ",";
    json += "\"activePowerL3\":" + String(data.activePowerL3, 2) + ",";
    json += "\"reactivePowerL3\":" + String(data.reactivePowerL3, 2) + ",";
    json += "\"powerFactorL3\":" + String(data.powerFactorL3, 3) + ",";
    json += "\"currentL3\":" + String(data.currentL3, 2) + ",";
    json += "\"frequencyL3\":" + String(data.frequencyL3, 2) + ",";
    json += "\"interruptionL3\":" + String(data.interruptionL3);

    json += "}";

    return json;
}