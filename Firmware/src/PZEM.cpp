#include "PZEM.h"

// ========================================
// KONFIGURASI
// ========================================

#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#define PZEM_BAUD_RATE 9600

// ========================================
// CONSTRUCTOR
// ========================================

PZEM::PZEM()
    : pzemSerial(2),
      pzem(pzemSerial, PZEM_RX_PIN, PZEM_TX_PIN),
      voltage(NAN),
      current(NAN),
      power(NAN),
      energy(NAN),
      frequency(NAN),
      pf(NAN),
      dataValid(false)
{
}

// ========================================
// INISIALISASI
// ========================================

void PZEM::begin()
{
    pzemSerial.begin(
        PZEM_BAUD_RATE,
        SERIAL_8N1,
        PZEM_RX_PIN,
        PZEM_TX_PIN
    );

    delay(1000);

    Serial.println("PZEM-004T initialized.");

    Serial.print("Custom Address: 0x");
    Serial.println(pzem.readAddress(), HEX);
}

// ========================================
// MEMBACA DATA
// ========================================

void PZEM::readData()
{
    voltage   = pzem.voltage();
    current   = pzem.current();
    power     = pzem.power();
    energy    = pzem.energy();
    frequency = pzem.frequency();
    pf        = pzem.pf();

    dataValid = !isnan(voltage)   &&
                !isnan(current)   &&
                !isnan(power)     &&
                !isnan(energy)    &&
                !isnan(frequency) &&
                !isnan(pf);
}

// ========================================
// MENAMPILKAN DATA
// ========================================

void PZEM::printData()
{
    if (!dataValid)
    {
        Serial.println("Error reading PZEM data.");
        Serial.println("Check wiring and communication.");
        return;
    }

    Serial.println("-------- DATA KELISTRIKAN --------");

    Serial.print("Voltage   : ");
    Serial.print(voltage, 1);
    Serial.println(" V");

    Serial.print("Current   : ");
    Serial.print(current, 3);
    Serial.println(" A");

    Serial.print("Power     : ");
    Serial.print(power, 1);
    Serial.println(" W");

    Serial.print("Energy    : ");
    Serial.print(energy, 3);
    Serial.println(" kWh");

    Serial.print("Frequency : ");
    Serial.print(frequency, 1);
    Serial.println(" Hz");

    Serial.print("Power Fact: ");
    Serial.println(pf, 2);

    Serial.println("----------------------------------");
    Serial.println();
}