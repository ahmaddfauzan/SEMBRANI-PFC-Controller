#include "PowerMeter3Phase.h"
#include <math.h>

// =====================================================
// KONSTRUKTOR
// =====================================================

PowerMeter3Phase::PowerMeter3Phase()
    : meterSerial(1) {
    resetData();
}

// =====================================================
// RESET DATA
// =====================================================

void PowerMeter3Phase::resetData() {
    data.voltageL1N = NAN;
    data.voltageL2N = NAN;
    data.voltageL3N = NAN;

    data.voltageL1L2 = NAN;
    data.voltageL2L3 = NAN;
    data.voltageL3L1 = NAN;

    data.currentL1 = NAN;
    data.currentL2 = NAN;
    data.currentL3 = NAN;

    data.activePower = NAN;
    data.reactivePower = NAN;
    data.apparentPower = NAN;

    data.powerFactor = NAN;
    data.frequency = NAN;
    data.energy = NAN;

    data.valid = false;
}

// =====================================================
// CALLBACK RS485
// =====================================================

void PowerMeter3Phase::preTransmission() {
    digitalWrite(DE_RE_PIN, HIGH);
}

void PowerMeter3Phase::postTransmission() {
    digitalWrite(DE_RE_PIN, LOW);
}

// =====================================================
// INISIALISASI
// =====================================================

void PowerMeter3Phase::begin() {
    pinMode(DE_RE_PIN, OUTPUT);
    digitalWrite(DE_RE_PIN, LOW);

    meterSerial.begin(
        BAUD_RATE,
        SERIAL_8E1,
        RX_PIN,
        TX_PIN
    );

    modbus.begin(SLAVE_ID, meterSerial);

    modbus.preTransmission(preTransmission);
    modbus.postTransmission(postTransmission);

    Serial.println("[PM1200] Initializing...");
    Serial.println("[PM1200] UART1 RX=26 TX=27");
    Serial.println("[PM1200] RS485 DE/RE=14");
    Serial.println("[PM1200] 9600 baud, 8E1, Slave ID=1");
}

// =====================================================
// MEMBACA FLOAT 32-BIT
// =====================================================

bool PowerMeter3Phase::readFloatRegister(
    uint16_t address,
    float &value
) {
    // Setiap nilai float menggunakan dua register 16-bit.
    uint8_t result = modbus.readHoldingRegisters(address, 2);

    if (result != ModbusMaster::ku8MBSuccess) {
        Serial.print("[PM1200] Read error at register ");
        Serial.print(address);
        Serial.print(", Modbus status=0x");
        Serial.println(result, HEX);

        value = NAN;
        return false;
    }

    // PM1200 menggunakan format little-endian float.
    // Register pertama menjadi word rendah.
    uint16_t lowWord  = modbus.getResponseBuffer(0);
    uint16_t highWord = modbus.getResponseBuffer(1);

    uint32_t raw =
        ((uint32_t)highWord << 16) |
        (uint32_t)lowWord;

    memcpy(&value, &raw, sizeof(value));

    if (!isfinite(value) || value == -999.0f) {
        value = NAN;
        return false;
    }

    return true;
}

// =====================================================
// MEMBACA SEMUA PARAMETER
// =====================================================

bool PowerMeter3Phase::readData() {
    resetData();

    bool ok = true;

    // Tegangan fase-ke-netral.
    ok &= readFloatRegister(3927, data.voltageL1N);
    ok &= readFloatRegister(3941, data.voltageL2N);
    ok &= readFloatRegister(3955, data.voltageL3N);

    // Tegangan antarfase.
    ok &= readFloatRegister(3925, data.voltageL1L2);
    ok &= readFloatRegister(3939, data.voltageL2L3);
    ok &= readFloatRegister(3953, data.voltageL3L1);

    // Arus masing-masing fase.
    ok &= readFloatRegister(3929, data.currentL1);
    ok &= readFloatRegister(3943, data.currentL2);
    ok &= readFloatRegister(3957, data.currentL3);

    // Parameter total.
    ok &= readFloatRegister(3903, data.activePower);
    ok &= readFloatRegister(3905, data.reactivePower);
    ok &= readFloatRegister(3901, data.apparentPower);

    ok &= readFloatRegister(3907, data.powerFactor);
    ok &= readFloatRegister(3915, data.frequency);

    // Forward active energy.
    ok &= readFloatRegister(3961, data.energy);

    data.valid = ok;

    return data.valid;
}

// =====================================================
// GETTER
// =====================================================

PowerMeter3PhaseData PowerMeter3Phase::getData() {
    return data;
}

// =====================================================
// SERIAL MONITOR
// =====================================================

void PowerMeter3Phase::printData() {
    Serial.println();
    Serial.println("========== PM1200 DATA ==========");

    if (!data.valid) {
        Serial.println("Status: INVALID / COMMUNICATION ERROR");
        Serial.println("Periksa komunikasi RS485 dan register.");
        Serial.println("=================================");
        return;
    }

    Serial.printf(
        "Voltage L1-N : %.2f V\n",
        data.voltageL1N
    );

    Serial.printf(
        "Voltage L2-N : %.2f V\n",
        data.voltageL2N
    );

    Serial.printf(
        "Voltage L3-N : %.2f V\n",
        data.voltageL3N
    );

    Serial.printf(
        "Voltage L1-L2: %.2f V\n",
        data.voltageL1L2
    );

    Serial.printf(
        "Voltage L2-L3: %.2f V\n",
        data.voltageL2L3
    );

    Serial.printf(
        "Voltage L3-L1: %.2f V\n",
        data.voltageL3L1
    );

    Serial.printf(
        "Current L1   : %.3f A\n",
        data.currentL1
    );

    Serial.printf(
        "Current L2   : %.3f A\n",
        data.currentL2
    );

    Serial.printf(
        "Current L3   : %.3f A\n",
        data.currentL3
    );

    Serial.printf(
        "Active Power : %.2f W\n",
        data.activePower
    );

    Serial.printf(
        "Reactive Pwr : %.2f var\n",
        data.reactivePower
    );

    Serial.printf(
        "Apparent Pwr : %.2f VA\n",
        data.apparentPower
    );

    Serial.printf(
        "Power Factor : %.3f\n",
        data.powerFactor
    );

    Serial.printf(
        "Frequency    : %.2f Hz\n",
        data.frequency
    );

    Serial.printf(
        "Active Energy: %.3f kWh\n",
        data.energy
    );

    Serial.println("=================================");
}