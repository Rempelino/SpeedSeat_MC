#include "Axis.h"
#include <EEPROM.h>
/*
POSITION = 0,
    MAX_POSITION = 1,
    HOMING_OFFSET = 2,
    ACCELLERATION = 3,
    MAX_SPEED = 6,
    HOMING_STATUS = 7,
    NEW_HOMING = 8,*/
void Axis::saveData()
{
    EEPROMAdress = AxisNumber * 1000;
    writeEEPROM(maxPosition);
    writeEEPROM(homingOffset);
    writeEEPROM(acceleration);
    writeEEPROM(maxSpeed);
}

void Axis::readData()
{
    EEPROMAdress = AxisNumber * 1000;
    readEEPROM(maxPosition);
    readEEPROM(homingOffset);
    readEEPROM(acceleration);
    readEEPROM(maxSpeed);
}

void Axis::writeEEPROM(unsigned long data)
{
    EEPROM.put(EEPROMAdress, data);
    EEPROMAdress += sizeof(data);
}

void Axis::writeEEPROM(unsigned int data)
{
    EEPROM.put(EEPROMAdress, data);
    EEPROMAdress += sizeof(data);
}

void Axis::readEEPROM(unsigned long &data)
{
    unsigned long d;
    EEPROM.get(EEPROMAdress, d);
    EEPROMAdress += sizeof(data);
    if (d == 0xFFFFFFFF)
    {
        return;
    }
    data = d;
}

void Axis::readEEPROM(unsigned int &data)
{
    unsigned int d;
    EEPROM.get(EEPROMAdress, d);
    EEPROMAdress += sizeof(data);
    if (d == 0xFFFF)
    {
        return;
    }
    data = d;
}