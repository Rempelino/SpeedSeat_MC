#ifndef AXIS_H
#define AXIS_H
#ifndef PROCESSOR_CYCLES_PER_MICROSECOND
#define PROCESSOR_CYCLES_PER_MICROSECOND 16UL
#define ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS 4UL
#define ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES (ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS * 1000UL * PROCESSOR_CYCLES_PER_MICROSECOND)
#endif
#include "Arduino.h"

enum movementType{movementFromZero, movementWithChangeOfDirection, movementExtension};

class Axxis
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
    unsigned long MaxPosition;
    unsigned long acceleration;
    unsigned int HomingOffset;

    volatile bool aktiv;
    bool accelerating;
    bool deccelerating;
    bool homingAbgeschlossen;
    bool changeOfDirection;
    volatile bool runningMinSpeed;
    unsigned long currentSpeed;

    unsigned long posStartDeccelerating;
    unsigned long sollPositionNachRichtungswechsel;
    volatile unsigned long istPosition;
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
    bool preventBadHoming = true;

    void TimerInitialisieren();
    unsigned long getStopPosition();
    bool stoppositionLiegtHinterSollposition(unsigned long);
    enum movementType getMovementType(unsigned long);
    void defaultMove(unsigned long);
    uint16_t getTimeTillNextStep();
    void recalculateAccelleration();
    bool digitalReadAverage(int);
    

public:
    Axxis(int pin_Direction,
          int Pin_Enable,
          int Pin_Trouble,
          int Pin_InPosition,
          int Pin_Endstop,
          int Pin_Permission,
          unsigned int StepPinNumber,
          unsigned short AxisNomber,
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
    void home();
    void move(unsigned long);
    void stopAxis();
    void startAxis();
    void newStep();
    int getSomeValue();
};

#endif