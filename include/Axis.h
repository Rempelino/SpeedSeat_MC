#ifndef AXIS_H
#define AXIS_H
#define MAX_AMOUNT_OF_AXIS 10

#include "Arduino.h"
#include "configuration.h"
enum TRACKING_TYPE
{
    _MC_FOCUS_ON_TRACKING,
    _MC_FOCUS_ON_SMOOTHENESS
};

enum PARAMETER_MODE
{
    _MC_PERMANENT,
    _MC_TEMPORARY
};

class Axis
{
private:
    enum movementType
    {
        movementFromZero,
        movementWithChangeOfDirection,
        movementExtension
    };

    enum homingStep
    {
        waitForAxisToStop,
        clearEndstop,
        driveToEndstop,
        moveFromEndstopAfterHoming,
        waitingForEndOfMovement
    };
    const bool focusOnTracking;
    const bool focusOnSmootheness = !focusOnTracking;
    bool direction = false;
    volatile bool decelerating;
    bool toggle;
    volatile bool changeOfDirection;
    unsigned tablePosition;
    unsigned long stepsPerMillimeter = 1;
    unsigned long istPosition;
    unsigned long sollPosition;
    unsigned long maxSpeed;
    unsigned long acceleration;
    unsigned long defaultAcceleration;
    unsigned long defaulMaxSpeed;
    unsigned long speedWhileHoming;
    unsigned long accelerationWhileHoming;
    volatile bool stopping = false;
    uint8_t step_Bit;
    volatile uint8_t *step_Port;
    const int Pin_Step;
    const int Pin_Direction;
    const int Pin_Enable;
    const int Pin_Endstop;
    const int Pin_Trouble;
    unsigned long maxPosition;
    unsigned long homingOffset;
    unsigned timeTillNextExecute;
    volatile bool active;
    volatile unsigned long timeTillNextSpeedUpdate;
    volatile unsigned long sollPositionNachRichtungswechsel;
    volatile unsigned requiredTimerPeriod;
    static const unsigned tableSize = 200;
    unsigned long accelerationRecalculationPeriod;
    unsigned long lastCommandPosition;
    unsigned AxisNumber;
    unsigned int table[tableSize];
    bool HardwareHasBeenInitialized = false;
    bool AxisIsLocked = false;
    bool softwareLimitsEnabled = true;
    volatile bool movingVelocity = false;
    volatile bool homingActive = false;
#ifdef NO_HARDWARE
    volatile bool AxisIsHomed = true;
#else
    volatile bool AxisIsHomed = false;
#endif
    volatile homingStep homingStep = waitForAxisToStop;
    static bool SteppingIsEnabled;
    unsigned long EEPROMAdress;
    bool isInitialized = false;
    unsigned short ErrorID = 0;
    volatile bool AxisHasError = false;
    static bool InterruptHasBeenSet;
    volatile unsigned long positionStartDecelerating;
    unsigned maximumTablePosition;

    void calculateSpeedPeriodTable();
    void writeTable();
    void defaultMove(unsigned long);
    unsigned long calculateBreakingDistance();
    enum movementType getMovementType(unsigned long);
    bool stoppositionLiegtHinterSollposition(unsigned long);
    unsigned long getStopPosition();
    unsigned speedToTimerPeriod(unsigned long);
    void calculateAccelerationRecalculationPeriod();
    void initializeHardware();
    void init(int, int);
    void executeHoming();
    void setTempSpeed(unsigned long);
    void resetSpeed();
    void setTempAcceleration(unsigned long);
    void resetAcceleration();
    void commandFinished();
    void moveRelativeInternal(unsigned long, bool);
    void moveVelocityInternal(unsigned long, bool);
    void moveAbsoluteInternal(unsigned long);
    unsigned long getSpeed();
    bool AxisIsReady();
    static void setInterrupt();
    void loadDefaultValues();

    void readData();
    void verifyData();
    void writeEEPROM(unsigned long);
    void writeEEPROM(unsigned int);
    void readEEPROM(unsigned long &);
    void readEEPROM(unsigned int &);

public:
    Axis(int Pin_Step, int Pin_Direction, int Pin_Enable, int Pin_Endstop, const int Pin_Trouble, TRACKING_TYPE trackingType);

    // public variable to access from Interrupt
    static unsigned durationSinceLastInterrupt;
    static unsigned TimerPeriod;
    static unsigned AxisCounter;
    static void (*ExecutePointer[MAX_AMOUNT_OF_AXIS])();
    static volatile bool analyzeWorkload;
    static volatile float workload;

    void disableSoftwareLimits();
    static void disableStepping();
    static bool digitalReadAverage(int, int averageingNumber = 10);

    static void enableStepping();
    void enableSoftwareLimits();
    void execute();

    unsigned long getIstpositon();
    unsigned long getSollpositon();
    unsigned long getHomingOffset();
    unsigned long getAcceleration();
    unsigned long getHomingSpeed();
    unsigned long getHomingAcceleration();
    unsigned long getMaxSpeed();
    unsigned getErrorID();
    bool getDirection();
    static float getWorkload();
    unsigned long getMaxPosition();

    bool hasError();
    void home();

    bool isRunningMaxSpeed();
    bool isActive();
    bool isHomed();
    bool isDeccelerating();

    void lock();

    void moveAbsolute(unsigned long);
    void moveAbsolute(unsigned long, unsigned long);
    void moveAbsoluteSteps(unsigned long);
    void moveRelative(unsigned long, bool);
    void moveVelocity(unsigned long, bool);

    void printIstPosition();
    void printSollPosition();
    void printMaxPosition();
    void printPositionDeccelerating();
    void printTimerPeriod();

    void resetAxis();

    void setAcceleration(unsigned long, PARAMETER_MODE parameterMode = _MC_PERMANENT);
    void setSpeed(unsigned long);
    void setMaxPosition(unsigned long);
    void setHomingOffset(unsigned long);
    void setHomingSpeed(unsigned long);
    void setHomingAcceleration(unsigned long);
    void setStepsPerMillimeter(unsigned long);
    void stop();
    static bool steppingIsEnabled();
    void saveData();

    void unlock();
};

#endif