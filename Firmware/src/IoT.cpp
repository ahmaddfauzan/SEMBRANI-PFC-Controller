#include "IoT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>

// =====================================================
// KONFIGURASI WI-FI
// =====================================================

static const char* WIFI_SSID = "R PRO JOGJA";
static const char* WIFI_PASSWORD = "wifigratis21";

// =====================================================
// KONFIGURASI MQTT
// =====================================================

static const char* MQTT_SERVER = "broker.emqx.io";
static const uint16_t MQTT_PORT = 1883;
static const char* MQTT_TOPIC = "sembrani/powermeter";

// =====================================================
// INTERVAL
// =====================================================

static const unsigned long WIFI_RETRY_INTERVAL = 10000;
static const unsigned long MQTT_RETRY_INTERVAL = 3000;
static const unsigned long STATUS_INTERVAL = 3000;

// =====================================================
// STATUS WI-FI
// =====================================================

static const char* getWiFiStatusText(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:
            return "Idle";

        case WL_NO_SSID_AVAIL:
            return "SSID not found";

        case WL_SCAN_COMPLETED:
            return "Scan completed";

        case WL_CONNECTED:
            return "Connected";

        case WL_CONNECT_FAILED:
            return "Connection failed";

        case WL_CONNECTION_LOST:
            return "Connection lost";

        case WL_DISCONNECTED:
            return "Disconnected";

        default:
            return "Unknown";
    }
}

// =====================================================
// CONSTRUCTOR
// =====================================================

IoT::IoT()
    : mqttClient(wifiClient),
      wifiSSID(WIFI_SSID),
      wifiPassword(WIFI_PASSWORD),
      mqttServer(MQTT_SERVER),
      mqttPort(MQTT_PORT),
      mqttTopic(MQTT_TOPIC),
      lastPublish(0),
      lastReconnectAttempt(0) {
}

// =====================================================
// INITIALISASI
// =====================================================

void IoT::begin() {
    Serial.println();
    Serial.println("================================");
    Serial.println("       SEMBRANI IoT");
    Serial.println("================================");

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    mqttClient.setServer(mqttServer, mqttPort);
    mqttClient.setBufferSize(1024);
    mqttClient.setKeepAlive(30);
    mqttClient.setSocketTimeout(5);

    Serial.print("[WiFi] SSID: ");
    Serial.println(wifiSSID);

    Serial.println("[IoT] Initialization complete.");

    connectWiFi();
}

// =====================================================
// KONEKSI WI-FI
// =====================================================

void IoT::connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    static unsigned long lastWiFiAttempt = 0;

    if (lastWiFiAttempt != 0 &&
        millis() - lastWiFiAttempt < WIFI_RETRY_INTERVAL) {
        return;
    }

    lastWiFiAttempt = millis();

    Serial.println();
    Serial.println("[WiFi] Starting connection...");
    Serial.print("[WiFi] SSID: ");
    Serial.println(wifiSSID);

    WiFi.disconnect();
    WiFi.begin(wifiSSID, wifiPassword);
}

// =====================================================
// KONEKSI MQTT
// =====================================================

void IoT::connectMQTT() {
    if (WiFi.status() != WL_CONNECTED ||
        mqttClient.connected()) {
        return;
    }

    if (millis() - lastReconnectAttempt <
        MQTT_RETRY_INTERVAL) {
        return;
    }

    lastReconnectAttempt = millis();

    String clientId = "SEMBRANI-ESP32-";
    clientId += String(
        static_cast<uint32_t>(ESP.getEfuseMac()),
        HEX
    );

    Serial.print("[MQTT] Connecting to ");
    Serial.print(mqttServer);
    Serial.print(":");
    Serial.println(mqttPort);

    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("[MQTT] Connected.");

        Serial.print("[MQTT] Topic: ");
        Serial.println(mqttTopic);
    } else {
        Serial.print("[MQTT] Connection failed. State: ");
        Serial.println(mqttClient.state());
    }
}

// =====================================================
// LOOP IOT
// =====================================================

void IoT::loop() {
    static unsigned long lastStatusPrint = 0;
    static bool previousWiFiConnected = false;
    static bool previousMQTTConnected = false;

    bool wifiConnected = (WiFi.status() == WL_CONNECTED);
    bool mqttConnected = mqttClient.connected();

    // Deteksi perubahan status Wi-Fi
    if (wifiConnected != previousWiFiConnected) {
        previousWiFiConnected = wifiConnected;

        if (wifiConnected) {
            Serial.println();
            Serial.println("[WiFi] Connected successfully.");

            Serial.print("[WiFi] IP address: ");
            Serial.println(WiFi.localIP());

            Serial.print("[WiFi] Signal strength: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
        } else {
            Serial.println("[WiFi] Connection lost.");
        }
    }

    // Wi-Fi belum terhubung
    if (!wifiConnected) {
        connectWiFi();

        if (millis() - lastStatusPrint >= STATUS_INTERVAL) {
            lastStatusPrint = millis();

            Serial.print("[WiFi] Status: ");
            Serial.println(
                getWiFiStatusText(WiFi.status())
            );
        }

        return;
    }

    // Wi-Fi terhubung, coba sambungkan MQTT
    if (!mqttClient.connected()) {
        connectMQTT();

        if (previousMQTTConnected) {
            Serial.println("[MQTT] Connection lost.");
            previousMQTTConnected = false;
        }

        return;
    }

    // Deteksi MQTT berhasil terhubung
    if (!previousMQTTConnected) {
        previousMQTTConnected = true;
        Serial.println("[MQTT] Communication ready.");
    }

    // Menjaga koneksi MQTT tetap aktif
    mqttClient.loop();
}

// =====================================================
// STATUS KONEKSI
// =====================================================

bool IoT::isConnected() {
    return WiFi.status() == WL_CONNECTED &&
           mqttClient.connected();
}

// =====================================================
// PUBLIKASI DATA PZEM
// =====================================================

void IoT::publishPowerData(
    float voltage,
    float current,
    float power,
    float energy,
    float frequency,
    float powerFactor
) {
    if (!isConnected()) {
        Serial.println(
            "[MQTT] Disconnected. Data not published."
        );
        return;
    }

    // Estimasi daya semu untuk pengukuran satu fase
    float apparentPower = voltage * current;

    char payload[768];

    snprintf(
        payload,
        sizeof(payload),
        "{"
            "\"activePower\":%.2f,"
            "\"reactivePower\":null,"
            "\"apparentPower\":%.2f,"
            "\"powerFactor\":%.3f,"
            "\"voltageLL\":null,"
            "\"voltageLN\":%.2f,"
            "\"current\":%.3f,"
            "\"frequency\":%.2f,"
            "\"energy\":%.3f,"
            "\"voltageL1N\":null,"
            "\"voltageL2N\":null,"
            "\"voltageL3N\":null,"
            "\"voltageL1L2\":null,"
            "\"voltageL2L3\":null,"
            "\"voltageL1L3\":null,"
            "\"apparentPowerL1\":null,"
            "\"activePowerL1\":null,"
            "\"reactivePowerL1\":null,"
            "\"powerFactorL1\":null,"
            "\"currentL1\":null,"
            "\"apparentPowerL2\":null,"
            "\"activePowerL2\":null,"
            "\"reactivePowerL2\":null,"
            "\"powerFactorL2\":null,"
            "\"currentL2\":null,"
            "\"apparentPowerL3\":null,"
            "\"activePowerL3\":null,"
            "\"reactivePowerL3\":null,"
            "\"powerFactorL3\":null,"
            "\"currentL3\":null"
        "}",
        power,
        apparentPower,
        powerFactor,
        voltage,
        current,
        frequency,
        energy
    );

    bool success = mqttClient.publish(
        mqttTopic,
        payload
    );

    if (success) {
        Serial.println("[MQTT] Publish successful:");
        Serial.println(payload);
    } else {
        Serial.println("[MQTT] Publish failed.");
    }
}