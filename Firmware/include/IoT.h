#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class IoT
{
public:
    IoT();

    void begin();
    void loop();

    bool isConnected();

    void publishPowerData(
        float voltage,
        float current,
        float power,
        float energy,
        float frequency,
        float powerFactor
    );

private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;

    const char* wifiSSID;
    const char* wifiPassword;

    const char* mqttServer;
    uint16_t mqttPort;
    const char* mqttTopic;

    unsigned long lastPublish;
    unsigned long lastReconnectAttempt;

    void connectWiFi();
    void connectMQTT();
};

#endif