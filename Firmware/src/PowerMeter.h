#ifndef POWER_METER_H
#define POWER_METER_H

#include <Arduino.h>
#include <ModbusMaster.h>

struct PowerData
{
    // =========================
    // TOTAL
    // =========================
    float apparentPower;      // VA
    float activePower;        // W
    float reactivePower;      // VAR
    float powerFactor;        // PF

    float voltageLL;          // Average V L-L
    float voltageLN;          // Average V L-N
    float current;            // Average current
    float frequency;          // Hz

    uint32_t interruptionCount;

    // =========================
    // VOLTAGE LINE-TO-NEUTRAL
    // =========================
    float voltageL1N;         // L1-N
    float voltageL2N;         // L2-N
    float voltageL3N;         // L3-N

    // =========================
    // VOLTAGE LINE-TO-LINE
    // =========================
    float voltageL1L2;        // L1-L2
    float voltageL2L3;        // L2-L3
    float voltageL1L3;        // L1-L3

    // =========================
    // PHASE 1
    // =========================
    float apparentPowerL1;    // VA1
    float activePowerL1;      // W1
    float reactivePowerL1;     // VAR1
    float powerFactorL1;      // PF1
    float currentL1;           // A1
    float frequencyL1;         // F1

    uint32_t interruptionL1;

    // =========================
    // PHASE 2
    // =========================
    float apparentPowerL2;    // VA2
    float activePowerL2;      // W2
    float reactivePowerL2;    // VAR2
    float powerFactorL2;      // PF2
    float currentL2;           // A2
    float frequencyL2;         // F2

    uint32_t interruptionL2;

    // =========================
    // PHASE 3
    // =========================
    float apparentPowerL3;    // VA3
    float activePowerL3;      // W3
    float reactivePowerL3;     // VAR3
    float powerFactorL3;      // PF3
    float currentL3;           // A3
    float frequencyL3;         // F3

    uint32_t interruptionL3;
};

class PowerMeter
{
public:

    PowerMeter(
        HardwareSerial &serial,
        uint8_t rxPin,
        uint8_t txPin,
        uint8_t controlPin,
        uint8_t slaveId = 1
    );

    void begin(uint32_t baudrate = 9600);

    bool read(PowerData &data);

    PowerData getData() const;

    uint8_t getLastError() const;

private:

    HardwareSerial &_serial;
    ModbusMaster _modbus;

    uint8_t _rxPin;
    uint8_t _txPin;
    uint8_t _controlPin;
    uint8_t _slaveId;

    uint8_t _lastError;

    PowerData _data;

    static PowerMeter *_instance;

    static void preTransmissionCallback();
    static void postTransmissionCallback();

    float registersToFloat(
        uint16_t lowWord,
        uint16_t highWord
    );

    uint32_t registersToUint32(
        uint16_t lowWord,
        uint16_t highWord
    );
};

#endif