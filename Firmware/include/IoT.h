#ifndef IOT_H
#define IOT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class IoT {
public:
    IoT();
    void begin();
    void loop();
    bool isConnected();

    // Mengambil perintah target PF yang sudah diterima MQTT.
    bool consumeTargetPF(float &targetPF);

    // Dipanggil main.cpp setelah target PF diterapkan ke variabel sistem.
    void publishTargetPFAck(float targetPF);

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

    volatile bool targetPFCommandPending;
    float requestedTargetPF;

    static IoT* callbackInstance;
    static void mqttMessageCallback(char* topic, byte* payload, unsigned int length);
    void handleMqttMessage(const char* topic, const byte* payload, unsigned int length);

    void connectWiFi();
    void connectMQTT();
};

#endif
