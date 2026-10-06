#include "PowerMeter.h"
#include <cstring>

PowerMeter *PowerMeter::_instance = nullptr;


// ============================================================
// CONSTRUCTOR
// ============================================================

PowerMeter::PowerMeter(
    HardwareSerial &serial,
    uint8_t rxPin,
    uint8_t txPin,
    uint8_t controlPin,
    uint8_t slaveId
)
    : _serial(serial),
      _rxPin(rxPin),
      _txPin(txPin),
      _controlPin(controlPin),
      _slaveId(slaveId),
      _lastError(0)
{
    _instance = this;
}


// ============================================================
// BEGIN
// ============================================================

void PowerMeter::begin(uint32_t baudrate)
{
    pinMode(_controlPin, OUTPUT);

    // Start in receive mode
    digitalWrite(_controlPin, LOW);

    // PM1200 default:
    // 9600 baud
    // 8 data bits
    // Even parity
    // 1 stop bit
    _serial.begin(
        baudrate,
        SERIAL_8E1,
        _rxPin,
        _txPin
    );

    _modbus.begin(_slaveId, _serial);

    _modbus.preTransmission(
        PowerMeter::preTransmissionCallback
    );

    _modbus.postTransmission(
        PowerMeter::postTransmissionCallback
    );
}


// ============================================================
// RS485 TRANSMISSION CONTROL
// ============================================================

void PowerMeter::preTransmissionCallback()
{
    if (_instance != nullptr)
    {
        digitalWrite(
            _instance->_controlPin,
            HIGH
        );
    }
}


void PowerMeter::postTransmissionCallback()
{
    if (_instance != nullptr)
    {
        digitalWrite(
            _instance->_controlPin,
            LOW
        );
    }
}


// ============================================================
// CONVERT 2 MODBUS REGISTERS TO FLOAT
// ============================================================

float PowerMeter::registersToFloat(
    uint16_t lowWord,
    uint16_t highWord
)
{
    uint32_t raw =
        ((uint32_t)highWord << 16) |
        lowWord;

    float value;

    memcpy(
        &value,
        &raw,
        sizeof(float)
    );

    return value;
}


// ============================================================
// CONVERT 2 MODBUS REGISTERS TO UINT32
// ============================================================

uint32_t PowerMeter::registersToUint32(
    uint16_t lowWord,
    uint16_t highWord
)
{
    return
        ((uint32_t)highWord << 16) |
        lowWord;
}


// ============================================================
// READ PM1200
// ============================================================

bool PowerMeter::read(PowerData &data)
{
    uint8_t response;


    // ========================================================
    // 1. TOTAL RMS BLOCK
    //
    // PM1200:
    // 3001 VA
    // 3003 W
    // 3005 VAR
    // 3007 PF
    // 3009 VLL
    // 3011 VLN
    // 3013 A
    // 3015 Frequency
    // 3019 Interruption
    //
    // ModbusMaster uses zero-based address:
    // 3001 -> 3000
    // ========================================================

    response = _modbus.readHoldingRegisters(
        3000,
        20
    );

    if (response != _modbus.ku8MBSuccess)
    {
        _lastError = response;
        return false;
    }

    data.apparentPower =
        registersToFloat(
            _modbus.getResponseBuffer(0),
            _modbus.getResponseBuffer(1)
        );

    data.activePower =
        registersToFloat(
            _modbus.getResponseBuffer(2),
            _modbus.getResponseBuffer(3)
        );

    data.reactivePower =
        registersToFloat(
            _modbus.getResponseBuffer(4),
            _modbus.getResponseBuffer(5)
        );

    data.powerFactor =
        registersToFloat(
            _modbus.getResponseBuffer(6),
            _modbus.getResponseBuffer(7)
        );

    data.voltageLL =
        registersToFloat(
            _modbus.getResponseBuffer(8),
            _modbus.getResponseBuffer(9)
        );

    data.voltageLN =
        registersToFloat(
            _modbus.getResponseBuffer(10),
            _modbus.getResponseBuffer(11)
        );

    data.current =
        registersToFloat(
            _modbus.getResponseBuffer(12),
            _modbus.getResponseBuffer(13)
        );

    data.frequency =
        registersToFloat(
            _modbus.getResponseBuffer(14),
            _modbus.getResponseBuffer(15)
        );

    data.interruptionCount =
        registersToUint32(
            _modbus.getResponseBuffer(18),
            _modbus.getResponseBuffer(19)
        );


    // ========================================================
    // 2. R PHASE / L1 RMS BLOCK
    //
    // 3031 VA1
    // 3033 W1
    // 3035 VAR1
    // 3037 PF1
    // 3039 V12 = L1-L2
    // 3041 V1  = L1-N
    // 3043 A1
    // 3045 F1
    // 3049 Intr1
    //
    // Start address 3031 -> 3030
    // ========================================================

    response = _modbus.readHoldingRegisters(
        3030,
        20
    );

    if (response != _modbus.ku8MBSuccess)
    {
        _lastError = response;
        return false;
    }

    data.apparentPowerL1 =
        registersToFloat(
            _modbus.getResponseBuffer(0),
            _modbus.getResponseBuffer(1)
        );

    data.activePowerL1 =
        registersToFloat(
            _modbus.getResponseBuffer(2),
            _modbus.getResponseBuffer(3)
        );

    data.reactivePowerL1 =
        registersToFloat(
            _modbus.getResponseBuffer(4),
            _modbus.getResponseBuffer(5)
        );

    data.powerFactorL1 =
        registersToFloat(
            _modbus.getResponseBuffer(6),
            _modbus.getResponseBuffer(7)
        );

    data.voltageL1L2 =
        registersToFloat(
            _modbus.getResponseBuffer(8),
            _modbus.getResponseBuffer(9)
        );

    data.voltageL1N =
        registersToFloat(
            _modbus.getResponseBuffer(10),
            _modbus.getResponseBuffer(11)
        );

    data.currentL1 =
        registersToFloat(
            _modbus.getResponseBuffer(12),
            _modbus.getResponseBuffer(13)
        );

    data.frequencyL1 =
        registersToFloat(
            _modbus.getResponseBuffer(14),
            _modbus.getResponseBuffer(15)
        );

    data.interruptionL1 =
        registersToUint32(
            _modbus.getResponseBuffer(18),
            _modbus.getResponseBuffer(19)
        );


    // ========================================================
    // 3. Y PHASE / L2 RMS BLOCK
    //
    // 3061 VA2
    // 3063 W2
    // 3065 VAR2
    // 3067 PF2
    // 3069 V23 = L2-L3
    // 3071 V2  = L2-N
    // 3073 A2
    // 3075 F2
    // 3079 Intr2
    //
    // Start address 3061 -> 3060
    // ========================================================

    response = _modbus.readHoldingRegisters(
        3060,
        20
    );

    if (response != _modbus.ku8MBSuccess)
    {
        _lastError = response;
        return false;
    }

    data.apparentPowerL2 =
        registersToFloat(
            _modbus.getResponseBuffer(0),
            _modbus.getResponseBuffer(1)
        );

    data.activePowerL2 =
        registersToFloat(
            _modbus.getResponseBuffer(2),
            _modbus.getResponseBuffer(3)
        );

    data.reactivePowerL2 =
        registersToFloat(
            _modbus.getResponseBuffer(4),
            _modbus.getResponseBuffer(5)
        );

    data.powerFactorL2 =
        registersToFloat(
            _modbus.getResponseBuffer(6),
            _modbus.getResponseBuffer(7)
        );

    data.voltageL2L3 =
        registersToFloat(
            _modbus.getResponseBuffer(8),
            _modbus.getResponseBuffer(9)
        );

    data.voltageL2N =
        registersToFloat(
            _modbus.getResponseBuffer(10),
            _modbus.getResponseBuffer(11)
        );

    data.currentL2 =
        registersToFloat(
            _modbus.getResponseBuffer(12),
            _modbus.getResponseBuffer(13)
        );

    data.frequencyL2 =
        registersToFloat(
            _modbus.getResponseBuffer(14),
            _modbus.getResponseBuffer(15)
        );

    data.interruptionL2 =
        registersToUint32(
            _modbus.getResponseBuffer(18),
            _modbus.getResponseBuffer(19)
        );


    // ========================================================
    // 4. B PHASE / L3 RMS BLOCK
    //
    // 3091 VA3
    // 3093 W3
    // 3095 VAR3
    // 3097 PF3
    // 3099 V31 = L3-L1
    // 3101 V3  = L3-N
    // 3103 A3
    // 3105 F3
    // 3109 Intr3
    //
    // Start address 3091 -> 3090
    // ========================================================

    response = _modbus.readHoldingRegisters(
        3090,
        20
    );

    if (response != _modbus.ku8MBSuccess)
    {
        _lastError = response;
        return false;
    }

    data.apparentPowerL3 =
        registersToFloat(
            _modbus.getResponseBuffer(0),
            _modbus.getResponseBuffer(1)
        );

    data.activePowerL3 =
        registersToFloat(
            _modbus.getResponseBuffer(2),
            _modbus.getResponseBuffer(3)
        );

    data.reactivePowerL3 =
        registersToFloat(
            _modbus.getResponseBuffer(4),
            _modbus.getResponseBuffer(5)
        );

    data.powerFactorL3 =
        registersToFloat(
            _modbus.getResponseBuffer(6),
            _modbus.getResponseBuffer(7)
        );

    data.voltageL1L3 =
        registersToFloat(
            _modbus.getResponseBuffer(8),
            _modbus.getResponseBuffer(9)
        );

    data.voltageL3N =
        registersToFloat(
            _modbus.getResponseBuffer(10),
            _modbus.getResponseBuffer(11)
        );

    data.currentL3 =
        registersToFloat(
            _modbus.getResponseBuffer(12),
            _modbus.getResponseBuffer(13)
        );

    data.frequencyL3 =
        registersToFloat(
            _modbus.getResponseBuffer(14),
            _modbus.getResponseBuffer(15)
        );

    data.interruptionL3 =
        registersToUint32(
            _modbus.getResponseBuffer(18),
            _modbus.getResponseBuffer(19)
        );


    // ========================================================
    // SAVE DATA
    // ========================================================

    _data = data;

    _lastError = _modbus.ku8MBSuccess;

    return true;
}


// ============================================================
// GET LAST DATA
// ============================================================

PowerData PowerMeter::getData() const
{
    return _data;
}


// ============================================================
// GET LAST ERROR
// ============================================================

uint8_t PowerMeter::getLastError() const
{
    return _lastError;
}