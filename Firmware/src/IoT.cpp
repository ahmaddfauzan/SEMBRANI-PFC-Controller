#include "IoT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <math.h>
#include "secrets.h"

// =====================================================
// KONFIGURASI MQTT
// =====================================================

static const char* MQTT_SERVER = "broker.emqx.io";
static const uint16_t MQTT_PORT = 1883;
static const char* MQTT_TOPIC = "sembrani/powermeter";
static const char* MQTT_CONTROL_TOPIC = "sembrani/control";
static const char* MQTT_CONTROL_ACK_TOPIC = "sembrani/control/ack";
static constexpr float TARGET_PF_MIN = 0.85f;
static constexpr float TARGET_PF_MAX = 0.99f;

IoT* IoT::callbackInstance = nullptr;

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
      wifiSSID(SEMBRANI_WIFI_SSID),
      wifiPassword(SEMBRANI_WIFI_PASSWORD),
      mqttServer(MQTT_SERVER),
      mqttPort(MQTT_PORT),
      mqttTopic(MQTT_TOPIC),
      lastPublish(0),
      lastReconnectAttempt(0),
      targetPFCommandPending(false),
      requestedTargetPF(0.95f) {
    callbackInstance = this;
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
    mqttClient.setCallback(IoT::mqttMessageCallback);

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

        if (mqttClient.subscribe(MQTT_CONTROL_TOPIC, 0)) {
            Serial.print("[MQTT] Subscribed: ");
            Serial.println(MQTT_CONTROL_TOPIC);
        } else {
            Serial.println("[MQTT] Control subscription failed.");
        }

        Serial.print("[MQTT] Telemetry topic: ");
        Serial.println(mqttTopic);
    } else {
        Serial.print("[MQTT] Connection failed. State: ");
        Serial.println(mqttClient.state());
    }
}

// =====================================================
// MQTT CALLBACK
// =====================================================

void IoT::mqttMessageCallback(char* topic, byte* payload, unsigned int length) {
    if (callbackInstance != nullptr) {
        callbackInstance->handleMqttMessage(topic, payload, length);
    }
}

void IoT::handleMqttMessage(const char* topic, const byte* payload, unsigned int length) {
    if (topic == nullptr || strcmp(topic, MQTT_CONTROL_TOPIC) != 0 || payload == nullptr) {
        return;
    }

    // Salin payload menjadi string yang diakhiri null.
    char message[256];
    if (length == 0 || length >= sizeof(message)) {
        Serial.println("[MQTT] Control payload rejected: invalid length.");
        return;
    }
    memcpy(message, payload, length);
    message[length] = '\0';

    // Format yang diterima: {"command":"set_target_pf","targetPF":0.95}
    if (strstr(message, "\"command\"") == nullptr ||
        strstr(message, "set_target_pf") == nullptr) {
        Serial.println("[MQTT] Control command rejected: unknown command.");
        return;
    }

    char* valueKey = strstr(message, "targetPF");
    if (valueKey == nullptr) {
        Serial.println("[MQTT] Control command rejected: targetPF missing.");
        return;
    }

    char* colon = strchr(valueKey, ':');
    if (colon == nullptr) return;

    char* endPointer = nullptr;
    float value = strtof(colon + 1, &endPointer);
    if (endPointer == colon + 1 || !isfinite(value) ||
        value < TARGET_PF_MIN || value > TARGET_PF_MAX) {
        Serial.println("[MQTT] Target PF rejected. Allowed range: 0.85 - 0.99.");
        return;
    }

    requestedTargetPF = value;
    targetPFCommandPending = true;

    Serial.print("[MQTT] Target PF command received: ");
    Serial.println(requestedTargetPF, 2);
}

bool IoT::consumeTargetPF(float &targetPF) {
    if (!targetPFCommandPending) return false;

    targetPF = requestedTargetPF;
    targetPFCommandPending = false;
    return true;
}

void IoT::publishTargetPFAck(float targetPF) {
    if (!isConnected()) {
        Serial.println("[MQTT] Cannot send target PF ACK: disconnected.");
        return;
    }

    char payload[160];
    snprintf(payload, sizeof(payload),
        "{\"status\":\"applied\",\"command\":\"set_target_pf\",\"targetPF\":%.2f}",
        targetPF);

    if (mqttClient.publish(MQTT_CONTROL_ACK_TOPIC, payload, false)) {
        Serial.print("[MQTT] Target PF ACK sent: ");
        Serial.println(payload);
    } else {
        Serial.println("[MQTT] Failed to publish target PF ACK.");
    }
}

// Kirim nilai aktual ESP32. Pesan retained membuat browser baru menerima
// target PF terakhir meskipun perubahan dilakukan sebelum dashboard dibuka.
void IoT::publishTargetPFState(float targetPF) {
    if (!isConnected()) {
        Serial.println("[MQTT] Cannot publish target PF state: disconnected.");
        return;
    }

    char payload[180];
    snprintf(payload, sizeof(payload),
        "{\"status\":\"state\",\"command\":\"set_target_pf\",\"source\":\"esp32\",\"targetPF\":%.2f}",
        targetPF);

    if (mqttClient.publish(MQTT_CONTROL_ACK_TOPIC, payload, true)) {
        Serial.print("[MQTT] Target PF state published: ");
        Serial.println(payload);
    } else {
        Serial.println("[MQTT] Failed to publish target PF state.");
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

    // ================================================
    // VALIDASI DATA
    // ================================================

    const bool validVI =
        isfinite(voltage) &&
        isfinite(current) &&
        voltage >= 0.0f &&
        current >= 0.0f;

    const bool validP = isfinite(power);

    // ================================================
    // PERHITUNGAN DAYA SEMU (S)
    // S = V x I
    // ================================================

    const float apparentPower =
        validVI ? voltage * current : NAN;

    // ================================================
    // PERHITUNGAN DAYA REAKTIF (Q)
    // Q = sqrt(S² - P²)
    // ================================================

    float reactivePower = NAN;

    bool reactivePowerValid =
        validVI &&
        validP &&
        isfinite(apparentPower) &&
        apparentPower >= 0.0f;

    if (reactivePowerValid) {

        const float pMagnitude = fabsf(power);

        float qSquared =
            apparentPower * apparentPower -
            pMagnitude * pMagnitude;

        const float tolerance =
            0.01f * apparentPower * apparentPower + 0.01f;

        // Koreksi selisih negatif kecil akibat pembulatan sensor
        if (qSquared < 0.0f && qSquared >= -tolerance) {
            qSquared = 0.0f;
        }

        if (qSquared >= 0.0f && isfinite(qSquared)) {
            reactivePower = sqrtf(qSquared);
        } else {
            reactivePowerValid = false;
        }
    }

    // ================================================
    // FORMAT DATA REACTIVE POWER
    // ================================================

    char reactivePowerText[24];

    if (reactivePowerValid && isfinite(reactivePower)) {
        snprintf(
            reactivePowerText,
            sizeof(reactivePowerText),
            "%.2f",
            reactivePower
        );
    } else {
        snprintf(
            reactivePowerText,
            sizeof(reactivePowerText),
            "null"
        );
    }

    // ================================================
    // FORMAT PAYLOAD MQTT
    // ================================================

    char payload[768];

    snprintf(
        payload,
        sizeof(payload),
        "{"
            "\"activePower\":%.2f,"
            "\"reactivePower\":%s,"
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
        reactivePowerText,
        isfinite(apparentPower) ? apparentPower : 0.0f,
        powerFactor,
        voltage,
        current,
        frequency,
        energy
    );

    // ================================================
    // PUBLIKASI MQTT
    // ================================================

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