#include "Axis.h"
Axis::Axis(const int Pin_Step, const int Pin_Direction, const int Pin_Enable, const int Pin_Trouble, const int Pin_Endstop, const TRACKING_TYPE trackingType)
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
    maxSpeed = 20000ul;
    acceleration = 20000ul;
    speedWhileHoming = 2000;
    accelerationWhileHoming = 2000;
    maxPosition = 0xFFFFFFFF;

    readData();
    saveData();

    defaultAcceleration = acceleration;
    defaulMaxSpeed = maxSpeed;



    calculateBreakingDistance();
    writeTable();
    step_Bit = digitalPinToBitMask(Pin_Step);
    step_Port = portOutputRegister(digitalPinToPort(Pin_Step));
    isInitialized = true;
}

void Axis::initializeHardware()
{
    if (HardwareHasBeenInitialized){
        return;
    }
    pinMode(Pin_Step, OUTPUT);
    pinMode(Pin_Direction, OUTPUT);
    pinMode(Pin_Enable, OUTPUT);
    pinMode(Pin_Endstop, INPUT_PULLUP);
    lock();
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    OCR3A = 65535;           // set interrupt timer to min frequency
    TCCR3B |= (1 << WGM12);  // CTC mode
    TCCR3B |= (1 << CS10);   // no prescaler 16 = 1 microsekunde @16Mhz Processor
    TIMSK3 |= (1 << OCIE1A); // enable timer compare interrupt
    HardwareHasBeenInitialized = true;
    SteppingIsEnabled = true;
}
