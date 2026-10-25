#ifndef POWER_METER_3_PHASE_H
#define POWER_METER_3_PHASE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ModbusMaster.h>

struct PowerMeter3PhaseData {
    float voltageL1N;
    float voltageL2N;
    float voltageL3N;

    float voltageL1L2;
    float voltageL2L3;
    float voltageL3L1;

    float currentL1;
    float currentL2;
    float currentL3;

    float activePower;
    float reactivePower;
    float apparentPower;

    float powerFactor;
    float frequency;
    float energy;

    bool valid;
};

class PowerMeter3Phase {
public:
    PowerMeter3Phase();

    void begin();
    bool readData();
    void printData();

    PowerMeter3PhaseData getData();

private:
    static constexpr uint8_t RX_PIN = 14;
    static constexpr uint8_t TX_PIN = 13;
    static constexpr uint8_t DE_RE_PIN = 21;

    static constexpr uint32_t BAUD_RATE = 9600;
    static constexpr uint8_t SLAVE_ID = 1;

    HardwareSerial meterSerial;
    ModbusMaster modbus;

    PowerMeter3PhaseData data;

    static void preTransmission();
    static void postTransmission();

    bool readFloatRegister(
        uint16_t address,
        float &value
    );

    void resetData();
};

#endif