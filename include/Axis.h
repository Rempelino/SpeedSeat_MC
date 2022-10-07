#ifndef AXIS_H
#define AXIS_H
#ifndef PROCESSOR_CYCLES_PER_MICROSECOND
#define PROCESSOR_CYCLES_PER_MICROSECOND 16UL
#define ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS 4UL
#define ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES (ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS * 1000UL * PROCESSOR_CYCLES_PER_MICROSECOND)
#endif
#include "Arduino.h"
#include "communication.h"
#include "configuration.h"

enum movementType
{
    movementFromZero,
    movementWithChangeOfDirection,
    movementExtension
};

class Axis
{
private:
    const int Pin_Direction;
    const int Pin_Enable;
    const int Pin_Trouble;
    const int Pin_InPosition;
    const int Pin_Endstop;
    const int Pin_Permission;
    const unsigned int StepPinNumber;
    const unsigned short int AxisNomber;
    unsigned long accelerationPerAccelerationRecalculation;
    unsigned long maxSpeed;
    unsigned long DistanzAbbremsenVonMaxSpeed;
    unsigned long acceleration;
    unsigned int HomingOffset;
    bool accelerating;
    bool deccelerating;
    bool changeOfDirection;
    bool timerHasBeenInitialized;
    volatile bool runningMinSpeed;
    unsigned long currentSpeed;
    unsigned long posStartDeccelerating;
    unsigned long sollPositionNachRichtungswechsel;
    unsigned long sollPosition;
    bool currentDirection = true;
    unsigned int RundungsFehlerProAccellerationCalculation = 1;
    unsigned int RundungsfehlerSumiert;
    unsigned long CyclesSinceLastAccelerationCalculation;
    volatile uint16_t *TimerPeriod;
    volatile uint8_t *Port;
    volatile uint8_t *OutputRegister;
    bool toggle;
    unsigned int stepPeriodInProcessorCycles = 65535;
    unsigned long stepsPerMillimeter;
    bool preventBadHoming = PREVENT_BAD_HOMING;
    bool killed;
    unsigned int ErrorID;
    bool locked = true;
    bool homingActive = false;
    bool isInitialzed = false;
    static unsigned short nextAxisNomber;
    unsigned long EEPROMAdress;

    unsigned long getStopPosition();
    bool stoppositionLiegtHinterSollposition(unsigned long);
    enum movementType getMovementType(unsigned long);
    void defaultMove(unsigned long);
    uint16_t getTimeTillNextStep();
    void recalculateAccelleration();
    bool digitalReadAverage(int);
    void TimerInitialisieren();
    void saveData();
    void readData();
    void writeEEPROM(unsigned long);
    void writeEEPROM(unsigned int);
    void readEEPROM(unsigned long &);
    void readEEPROM(unsigned int &);

public:
    
    volatile bool aktiv;
    unsigned long MaxPosition;
    volatile unsigned long istPosition;
    bool isHomed;

    Axis(int pin_Direction,
         int Pin_Enable,
         int Pin_Trouble,
         int Pin_InPosition,
         int Pin_Endstop,
         int Pin_Permission,
         unsigned int StepPinNumber,
         unsigned long MaxPositionInMillimeter,
         unsigned long StepsPerMillimeter,
         unsigned long acceleration,
         unsigned long maxSpeed,
         unsigned int HomingOffset,
         volatile uint16_t *TimerPeriod,
         volatile uint8_t *Port,
         volatile uint8_t *OutputRegister);

    void printInputStatus();
    void dumpAxisParameter();
    void printStatus();
    void home();
    void move(unsigned long);
    void stopAxis();
    void startAxis();
    void newStep();
    bool hasError();
    unsigned int getErrorID();
    int getSomeValue();
    void lock();
    void unlock();
    bool isLocked();
    void setMaxPosition(unsigned long MaxPosition);
    void setHomingOffset(unsigned long offset);
    void setAcceleration(unsigned long acceleration);
    void setMaxSpeed(unsigned long maxSpeed);
};

#endif