#include "Axis.h"
#include "AxisConstructos.h"
#include "AxisMove.h"
#include "AxisHome.h"
#include "AxisStepping.h"
#include "AxisEEPROM.h"
#include "configuration.h"

#ifdef AUTO_SAVE
#define SAVE_DATA saveData();
#else
#define SAVE_DATA // NOTHING
#endif

unsigned Axis::AxisCounter = 0;
unsigned Axis::durationSinceLastInterrupt = 0;
unsigned Axis::TimerPeriod = 0xFFFF;
bool Axis::SteppingIsEnabled = false;
bool Axis::InterruptHasBeenSet = false;
volatile bool Axis::analyzeWorkload = false;
volatile float Axis::workload = 0;
void (*Axis::ExecutePointer[MAX_AMOUNT_OF_AXIS])() = {0};

unsigned long Axis::getIstpositon()
{
    return istPosition / stepsPerMillimeter;
}

unsigned long Axis::getMaxPosition()
{
    return maxPosition / stepsPerMillimeter;
}

unsigned long Axis::getSollpositon()
{
    return lastCommandPosition / stepsPerMillimeter;
}

unsigned long Axis::getHomingOffset()
{
    return homingOffset / stepsPerMillimeter;
}

unsigned long Axis::getAcceleration()
{
    return defaultAcceleration / stepsPerMillimeter;
}

unsigned long Axis::getMaxSpeed()
{
    return defaulMaxSpeed / stepsPerMillimeter;
}

unsigned long Axis::getSpeed()
{
    return F_CPU / table[tablePosition];
}

unsigned long Axis::getHomingSpeed()
{
    return speedWhileHoming / stepsPerMillimeter;
}

unsigned long Axis::getHomingAcceleration()
{
    return accelerationWhileHoming / stepsPerMillimeter;
}

bool Axis::isActive()
{
    return active;
}

bool Axis::getDirection()
{
    return direction;
}

bool Axis::isDeccelerating()
{
    return decelerating;
}

void Axis::writeTable()
{
    calculateAccelerationRecalculationPeriod();
    unsigned long long time = 0;
    unsigned long TimerPeriod = 0xFFFF;
    bool maximumTablePositionKnown = false;

    // Die erste Geschwindigkeit herrausfinden bei der die Steppdauer nicht die maximale Stepdauer eines Steps überschreitet
    while (TimerPeriod == 0xFFFF)
    {
        time += accelerationRecalculationPeriod;
        unsigned long speed = acceleration * time / F_CPU;
        if (speed > 0)
        {
            TimerPeriod = speedToTimerPeriod(speed);
            if (TimerPeriod > 0xFFFF)
            {
                TimerPeriod = 0xFFFF;
            }
        }
    }
    if (time > 0)
    {
        time -= accelerationRecalculationPeriod;
    }
    this->table[0] = 0xFFFF;
    for (unsigned int i = 1; i < tableSize; i++)
    {
        time += accelerationRecalculationPeriod;
        unsigned long speed = acceleration * time / F_CPU;
        if (!maximumTablePositionKnown)
        {
            if (speed > maxSpeed)
            {
                maximumTablePositionKnown = true;
                maximumTablePosition = i - 1;
                return;
            }
        }
        if (speed > 0)
        {
            TimerPeriod = speedToTimerPeriod(speed);
            if (TimerPeriod > 0xFFFF)
            {
                TimerPeriod = 0xFFFF;
            }
        }
        this->table[i] = (unsigned int)(TimerPeriod);
    }
    maximumTablePosition = tableSize - 1;
}

unsigned long Axis::calculateBreakingDistance()
{
    return (maxSpeed * maxSpeed) / (2 * acceleration);
}

bool Axis::isRunningMaxSpeed()
{
    return tablePosition == maximumTablePosition;
}

unsigned Axis::speedToTimerPeriod(unsigned long speed)
{
    unsigned long TimerPeriod = F_CPU / speed;
    if (TimerPeriod > 0xFFFF)
    {
        return 0xFFFF;
    }
    else
    {
        return TimerPeriod;
    }
}

void Axis::printIstPosition()
{
    Serial.print(AxisNumber);
    Serial.print("_Ist:");
    Serial.print(getIstpositon());
    Serial.print(",");
}

void Axis::printTimerPeriod()
{
    Serial.print(AxisNumber);
    Serial.print("_TimerPeriod:");
    Serial.print(requiredTimerPeriod);
    Serial.print(",");
}

void Axis::printSollPosition()
{
    Serial.print(AxisNumber);
    Serial.print("_Soll:");
    Serial.print(getSollpositon());
    Serial.print(",");
}

void Axis::printMaxPosition()
{
    Serial.print(AxisNumber);
    Serial.print("_Max:");
    Serial.print(getMaxPosition());
    Serial.print(",");
}

void Axis::printPositionDeccelerating()
{
    Serial.print(AxisNumber);
    Serial.print("_PosDec:");
    Serial.print(positionStartDecelerating / stepsPerMillimeter);
    Serial.print(",");
}

float Axis::getWorkload()
{
    return workload;
    if (!SteppingIsEnabled)
    {
        return 0;
    }
    unsigned long maxInterruptTime = 10;
    Axis::analyzeWorkload = true;
    unsigned long timeStampStartAnalyze = millis();
    while (analyzeWorkload)
    {
        if (millis() - timeStampStartAnalyze > maxInterruptTime)
        {
            return 0;
        }
    }
    return workload;
}

void Axis::calculateAccelerationRecalculationPeriod()
{
    // t = v/a
    accelerationRecalculationPeriod = (unsigned long)(unsigned long long)(maxSpeed) * (unsigned long long)(F_CPU) / acceleration / (tableSize - 1);
}

void Axis::setTempAcceleration(unsigned long acceleration)
{
    this->acceleration = acceleration;
    writeTable();
}

void Axis::resetAcceleration()
{
    if (acceleration == defaultAcceleration)
    {
        return;
    }
    acceleration = defaultAcceleration;
    writeTable();
}

void Axis::setAcceleration(unsigned long acceleration, PARAMETER_MODE parameterMode)
{
    if (parameterMode == _MC_PERMANENT)
    {
        this->acceleration = acceleration * stepsPerMillimeter;
        this->defaultAcceleration = this->acceleration;
        SAVE_DATA
        writeTable();
    }else{
        setTempAcceleration(acceleration * stepsPerMillimeter);
    }
}

void Axis::setHomingSpeed(unsigned long speed)
{
    this->speedWhileHoming = speed * stepsPerMillimeter;
    SAVE_DATA
}

void Axis::setHomingAcceleration(unsigned long acceleration)
{
    this->accelerationWhileHoming = acceleration * stepsPerMillimeter;
    SAVE_DATA
}

void Axis::setTempSpeed(unsigned long speed)
{
    this->maxSpeed = speed;
    writeTable();
}

void Axis::resetSpeed()
{
    if (maxSpeed == defaulMaxSpeed)
    {
        return;
    }
    maxSpeed = defaulMaxSpeed;
    writeTable();
}

void Axis::setSpeed(unsigned long maxSpeed)
{
    this->maxSpeed = maxSpeed * stepsPerMillimeter;
    this->defaulMaxSpeed = this->maxSpeed;
    SAVE_DATA
    writeTable();
}

void Axis::lock()
{
    digitalWrite(Pin_Enable, LOW);
    AxisIsLocked = true;
}

void Axis::unlock()
{
    digitalWrite(Pin_Enable, HIGH);
    active = false;
    stopping = false;
    AxisIsHomed = false;
    AxisIsLocked = false;
}

void Axis::enableSoftwareLimits()
{
    softwareLimitsEnabled = true;
}

void Axis::disableSoftwareLimits()
{
    softwareLimitsEnabled = false;
}

bool Axis::isHomed()
{
    return AxisIsHomed;
}

void Axis::setMaxPosition(unsigned long MaxPosition)
{
    unsigned long maxPosition = MaxPosition * stepsPerMillimeter;
    if (this->maxPosition != maxPosition)
    {
        unsigned long newPosition = istPosition * maxPosition / this->maxPosition;
        this->maxPosition = maxPosition;

        if (isInitialized)
        {
            SAVE_DATA
            if (AxisIsHomed)
            {
                moveAbsoluteInternal(newPosition);
            }
        }
    }
}

void Axis::setHomingOffset(unsigned long offset)
{
    offset = offset * stepsPerMillimeter;
    if (offset != homingOffset)
    {
        maxPosition = maxPosition + homingOffset - offset;
        homingOffset = offset;
        SAVE_DATA
        home();
    }
}

void Axis::setStepsPerMillimeter(unsigned long steps)
{
    stepsPerMillimeter = steps;
}

bool Axis::hasError()
{
    if (!HardwareHasBeenInitialized)
    {
        return false;
    }
    if (AxisHasError)
    {
        return true;
    }
    unsigned long int timeStamp;
    const unsigned long int timeTillError = 50; // time delay to prevent errors due to noise

    if (AxisIsHomed && !digitalRead(Pin_Endstop))
    {
        timeStamp = millis();
        while (!digitalRead(Pin_Endstop))
        {
            if (millis() - timeStamp >= timeTillError)
            {
                ErrorID = 3;
                AxisHasError = true;
                return true;
            }
        }
    }

    if (!digitalRead(Pin_Trouble))
    {
        timeStamp = millis();
        while (!digitalRead(Pin_Trouble))
        {
            if (millis() - timeStamp >= timeTillError)
            {
                ErrorID = 4;
                AxisHasError = true;
                return true;
            }
        }
    }
    return false;
}

void Axis::resetAxis()
{
    AxisHasError = false;
}

unsigned Axis::getErrorID()
{
    return ErrorID;
}

bool Axis::steppingIsEnabled()
{
    return SteppingIsEnabled;
}

bool Axis::AxisIsReady()
{
    if (!HardwareHasBeenInitialized)
    {
        initializeHardware();
    }
    if (!SteppingIsEnabled)
    {
        return false;
    }
    if (!AxisIsLocked)
    {
        return false;
    }
    return true;
}

bool Axis::digitalReadAverage(int pin, int averageingNumber)
{
    //read a digital pin and return the average result to prevent action due to noise
    int y = 0;
    int x = 0;
    for (x = 0; x < averageingNumber; x++)
    {
        if (digitalRead(pin))
        {
            y++;
        }
    }
    if (y > averageingNumber / 2)
    {
        return true;
    }
    else
    {
        return false;
    }
}