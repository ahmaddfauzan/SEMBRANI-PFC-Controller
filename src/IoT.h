#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "PowerMeter.h"

class IoT {
public:
    IoT(
        const char* ssid,
        const char* password,
        const char* mqttServer,
        uint16_t mqttPort,
        const char* mqttTopic
    );

    void begin();
    void loop();

    bool publishPowerData(const PowerData& data);
    bool isConnected();

private:
    const char* _ssid;
    const char* _password;

    const char* _mqttServer;
    uint16_t _mqttPort;
    const char* _mqttTopic;

    WiFiClient _wifiClient;
    PubSubClient _mqtt;

    void connectWiFi();
    void connectMQTT();

    String powerDataToJson(const PowerData& data);
};

#endif