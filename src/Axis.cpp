#include "Axis.h"
#include "AxisMove.h"
#include "AxisTimer.h"
#include "AxisSteps.h"
#include "AxisEEPROM.h"
unsigned short Axis::nextAxisNomber = 0;
Axis::Axis(int Pin_Direction,
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
           volatile uint8_t *OutputRegister)
    : Pin_Direction(Pin_Direction),
      Pin_Enable(Pin_Enable),
      Pin_Trouble(Pin_Trouble),
      Pin_InPosition(Pin_InPosition),
      Pin_Endstop(Pin_Endstop),
      Pin_Permission(Pin_Permission),
      StepPinNumber(StepPinNumber),
      AxisNomber(nextAxisNomber)
{

    this->MaxPosition = MaxPositionInMillimeter * StepsPerMillimeter;
    this->HomingOffset = HomingOffset;
    this->TimerPeriod = TimerPeriod;
    this->Port = Port;
    this->OutputRegister = OutputRegister;
    this->stepsPerMillimeter = StepsPerMillimeter;
    setAcceleration(acceleration);
    setMaxSpeed(maxSpeed);
    pinMode(Pin_Direction, OUTPUT);
    pinMode(Pin_Enable, OUTPUT);
    pinMode(Pin_InPosition, INPUT_PULLUP);
    pinMode(Pin_Trouble, INPUT_PULLUP);
    pinMode(Pin_Endstop, INPUT_PULLUP);

    digitalWrite(Pin_Direction, LOW);
    digitalWrite(Pin_Enable, LOW);
    uint8_t AktuellerWertPort = *OutputRegister;
    *OutputRegister = AktuellerWertPort | (1 << StepPinNumber);
    nextAxisNomber++;
    readData();
    saveData();
    isInitialzed = true;
}

void Axis::lock()
{
    if (!locked)
    {
        digitalWrite(Pin_Enable, LOW);
        locked = true;
        isHomed = false;
        killed = false;
        ErrorID = 0;
        home();
    }
}

void Axis::unlock()
{
    stopAxis();
    locked = false;
    digitalWrite(Pin_Enable, HIGH);
}

bool Axis::isLocked()
{
    return locked;
}

void Axis::home()
{
    isHomed = false;
    homingActive = true;
    if (SIMULATION)
    {
        Serial.print("Homing Axis: ");
        Serial.println(AxisNomber);
    }
    stopAxis();
    digitalWrite(Pin_Direction, HIGH);
    unsigned long myMicros;
    unsigned long microsLastCycle = micros();
    unsigned long mycrosSinceLastExecute = 0;
    const unsigned long intervall = 20 * stepsPerMillimeter;
    bool AchseHatEndstopVerlassen = false;
    while (!isHomed)
    {
        myMicros = micros();
        mycrosSinceLastExecute = mycrosSinceLastExecute + (myMicros - microsLastCycle);
        microsLastCycle = myMicros;
        if (mycrosSinceLastExecute > intervall)
        {
            mycrosSinceLastExecute = mycrosSinceLastExecute - intervall;

            if (!AchseHatEndstopVerlassen)
            {

                while (digitalRead(Pin_Permission))
                {
                    microsLastCycle = micros();
                    if (SIMULATION)
                    {
                        Serial.print("warte auf enable Pin: ");
                        Serial.println(Pin_Permission);
                    }
                }
                if (!digitalReadAverage(Pin_Endstop))
                {

                    uint8_t AktuellerWertPort = *Port;
                    if (toggle)
                    {
                        *Port = AktuellerWertPort | (1 << StepPinNumber);
                        toggle = false;
                    }
                    else
                    {
                        *Port = AktuellerWertPort & ~(1 << StepPinNumber);
                        toggle = true;
                    }
                }
                if (digitalReadAverage(Pin_Endstop))
                {
                    AchseHatEndstopVerlassen = true;
                    digitalWrite(Pin_Direction, LOW);
                }
            }
            else
            {

                while (digitalRead(Pin_Permission))
                {
                    microsLastCycle = micros();
                }

                if (!digitalReadAverage(Pin_Endstop) & !isHomed)
                {
                    isHomed = true;
                    istPosition = 0;
                }
                if (!isHomed)
                {
                    uint8_t AktuellerWertPort = *Port;
                    if (toggle)
                    {
                        *Port = AktuellerWertPort | (1 << StepPinNumber); // Axis->StepPinNumber;//*Axis->Port|(1<<Axis->StepPinNumber);
                        toggle = false;
                    }
                    else
                    {
                        *Port = AktuellerWertPort & ~(1 << StepPinNumber); //*Axis->Port|~(0<<Axis->StepPinNumber);
                        toggle = true;
                    }
                }
            }
        }
    }

    if (preventBadHoming)
    {
        istPosition = 1 * stepsPerMillimeter;
        while (aktiv)
        {
        }
        move(0);
        unsigned long timeStamp = millis();
        while (millis() - timeStamp < 30)
        {
            if (digitalReadAverage(Pin_Endstop))
            {
                return;
                homingActive = false;
            }
        }
    }

    move(HomingOffset * stepsPerMillimeter);

    while (aktiv)
    {
    }

    istPosition = 0;
    isHomed = true;
    homingActive = false;
}

void Axis::setMaxPosition(unsigned long MaxPosition)
{
    if (this->MaxPosition != MaxPosition)
    {
        unsigned long newPosition = istPosition * MaxPosition / this->MaxPosition;
        this->MaxPosition = MaxPosition;
        
        if (isInitialzed)
        {   
            saveData();
            move(newPosition);
        }
    }
}

void Axis::setHomingOffset(unsigned long offset)
{
    if (offset != HomingOffset)
    {
        HomingOffset = offset;
        saveData();
        if (!NO_HARDWARE)
        {
            home();
        }
    }
}

void Axis::setMaxSpeed(unsigned long MaxSpeed)
{
    this->maxSpeed = MaxSpeed * stepsPerMillimeter;
    this->DistanzAbbremsenVonMaxSpeed = (this->maxSpeed * this->maxSpeed) / (2 * this->acceleration);
    if (isInitialzed){
        saveData();
    }
    
}

void Axis::setAcceleration(unsigned long acceleration)
{
    this->acceleration = acceleration * stepsPerMillimeter;
    this->accelerationPerAccelerationRecalculation = this->acceleration / PROCESSOR_CYCLES_PER_MICROSECOND * ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES / 1000000;
    this->DistanzAbbremsenVonMaxSpeed = (this->maxSpeed * this->maxSpeed) / (2 * this->acceleration);
    if (isInitialzed){
        saveData();
    }
}

int Axis::getSomeValue()
{
    return HomingOffset;
}

void Axis::printInputStatus()
{
    Serial.println();
    if (AxisNomber == 0)
    {
        Serial.println("--------------------INPUTSTATUS X Achse-------------------------");
    }
    if (AxisNomber == 1)
    {
        Serial.println("--------------------INPUTSTATUS Y Achse-------------------------");
    }
    if (AxisNomber == 2)
    {
        Serial.println("--------------------INPUTSTATUS Z Achse-------------------------");
    }

    Serial.print("Endstop:");
    Serial.println(digitalRead(Pin_Endstop));

    Serial.print("InPosition:");
    Serial.println(digitalRead(Pin_InPosition));

    Serial.print("Trouble:");
    Serial.println(digitalRead(Pin_Trouble));
}

void Axis::dumpAxisParameter()
{
    Serial.println();

    if (AxisNomber == 0)
    {
        Serial.println("Parameter of X Axis:");
    }
    if (AxisNomber == 1)
    {
        Serial.println("Parameter of Y Axis:");
    }
    if (AxisNomber == 2)
    {
        Serial.println("Parameter of Z Axis:");
    }

    Serial.println("Unit -> Steps ---------------------------------");

    Serial.print("Accelleration: ");
    Serial.println(acceleration);

    Serial.print("accelerationPerAccelerationRecalculation: ");
    Serial.println(accelerationPerAccelerationRecalculation);

    Serial.print("MaxPosition: ");
    Serial.println(MaxPosition);

    Serial.println("Unit -> Millimeter ----------------------------");

    Serial.print("Accelleration: ");
    Serial.println(acceleration / stepsPerMillimeter);

    Serial.print("MaxPosition: ");
    Serial.println(MaxPosition / stepsPerMillimeter);

    Serial.println();
    Serial.println();
}

void Axis::printStatus()
{
    Serial.println();

    if (AxisNomber == 0)
    {
        Serial.println("Status of X Axis:");
    }
    if (AxisNomber == 1)
    {
        Serial.println("Status of Y Axis:");
    }
    if (AxisNomber == 2)
    {
        Serial.println("Status of Z Axis:");
    }

    Serial.println("Unit -> Steps ---------------------------------");

    Serial.print("Istposition: ");
    Serial.println(istPosition);

    Serial.print("Sollposition: ");
    Serial.println(sollPosition);

    Serial.print("Speed: ");
    Serial.println(currentSpeed);

    Serial.println("Unit -> Millimeter ----------------------------");

    Serial.print("Istposition: ");
    Serial.println(istPosition / stepsPerMillimeter);

    Serial.print("Sollposition: ");
    Serial.println(sollPosition / stepsPerMillimeter);

    Serial.print("Speed: ");
    Serial.println(currentSpeed / stepsPerMillimeter);

    Serial.println();
    Serial.println();
}

bool Axis::digitalReadAverage(int pin)
{
    int averageingNumber = 10;
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

bool Axis::hasError()
{
    unsigned long int timeStamp;
    const unsigned long int timeTillError = 50;//time delay to prevent errors due to noise
    if (killed)
    {
        return true;
    }

    if (isHomed & !digitalRead(Pin_Endstop))
    {
        timeStamp = millis();
        while (!digitalRead(Pin_Endstop))
        {
            if (millis() - timeStamp >= timeTillError)
            {
                stopAxis();
                killed = true;
                ErrorID = 3;
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
                stopAxis();
                killed = true;
                ErrorID = 4;
                return true;
                break;
            }
        }
    }
    return false;
}

unsigned int Axis::getErrorID()
{
    return ErrorID;
}