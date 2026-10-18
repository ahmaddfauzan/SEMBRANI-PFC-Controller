#ifndef PZEM_H
#define PZEM_H

#include <Arduino.h>
#include <PZEM004Tv30.h>

class PZEM {
public:
    PZEM();

    void begin();
    void readData();
    void printData();

    // Getter untuk mengambil hasil pengukuran
    float getVoltage();
    float getCurrent();
    float getPower();
    float getEnergy();
    float getFrequency();
    float getPF();

    bool isDataValid();

private:
    HardwareSerial pzemSerial;
    PZEM004Tv30 pzem;

    float voltage;
    float current;
    float power;
    float energy;
    float frequency;
    float pf;

    bool dataValid;
};

#endif