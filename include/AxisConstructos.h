#include "configuration.h"
#include "Axis.h"
Axis::Axis(const int Pin_Step, const int Pin_Direction, const int Pin_Enable, const int Pin_Endstop, const int Pin_Trouble, const TRACKING_TYPE trackingType)
    : focusOnTracking(trackingType == _MC_FOCUS_ON_TRACKING),
      Pin_Step(Pin_Step),
      Pin_Direction(Pin_Direction),
      Pin_Enable(Pin_Enable),
      Pin_Endstop(Pin_Endstop),
      Pin_Trouble(Pin_Trouble)
{
    init(Pin_Step, Pin_Direction);
}

void Axis::init(int Pin_Step, int Pin_Direction)
{
    AxisNumber = AxisCounter;
    AxisCounter++;
    loadDefaultValues();
#ifdef USE_EEPROM
    readData();
#endif
    verifyData();
    saveData();
    calculateBreakingDistance();
    writeTable();
    step_Bit = digitalPinToBitMask(Pin_Step);
    step_Port = portOutputRegister(digitalPinToPort(Pin_Step));
    isInitialized = true;
}

void Axis::initializeHardware()
{
    if (HardwareHasBeenInitialized)
    {
        return;
    }
    pinMode(Pin_Step, OUTPUT);
    pinMode(Pin_Direction, OUTPUT);
    pinMode(Pin_Enable, OUTPUT);
    pinMode(Pin_Endstop, INPUT_PULLUP);
    pinMode(Pin_Trouble, INPUT_PULLUP);
    lock();
    if (!InterruptHasBeenSet)
    {
        setInterrupt();
    }
    HardwareHasBeenInitialized = true;
}

void Axis::loadDefaultValues()
{
    defaulMaxSpeed = 14000ul;
    defaultAcceleration = 40000ul;
    speedWhileHoming = 1000ul;
    accelerationWhileHoming = 40000ul;
    maxPosition = 8400;
    acceleration = defaultAcceleration;
    maxSpeed = defaulMaxSpeed;
}

void Axis::verifyData()
{
    bool somethingWasChanged = false;
    if (acceleration == 0)
    {
        acceleration = 1;
        somethingWasChanged = true;
    }
    if (defaultAcceleration == 0)
    {
        defaultAcceleration = 1;
        somethingWasChanged = true;
    }
    if (accelerationWhileHoming == 0)
    {
        accelerationWhileHoming = 1;
        somethingWasChanged = true;
    }
    if (maxSpeed == 0)
    {
        maxSpeed = 1;
        somethingWasChanged = true;
    }
    if (defaulMaxSpeed == 0)
    {
        defaulMaxSpeed = 1;
        somethingWasChanged = true;
    }
    if (speedWhileHoming == 0)
    {
        speedWhileHoming = 1;
        somethingWasChanged = true;
    }

    if (somethingWasChanged)
    {
        calculateBreakingDistance();
        writeTable();
    }
}

void Axis::setInterrupt()
{
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    OCR3A = 65535;                    // set interrupt timer to min frequency
    TCCR3B |= (1 << WGM12);           // CTC mode
    TCCR3B |= (1 << CS10);            // no prescaler 16 = 1 microsekunde @16Mhz Processor
    TIMSK3 = TIMSK3 & ~(1 << OCIE1A); // disable timer compare interrupt
    InterruptHasBeenSet = true;
    SteppingIsEnabled = false;
}
