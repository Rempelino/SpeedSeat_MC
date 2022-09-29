#include "Axxis.h"
#include "AxxisMove.h"
#include "AxxisTimer.h"
#include "AxxisSteps.h"

Axxis::Axxis(int Pin_Direction,
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
             volatile uint8_t *OutputRegister)
    : Pin_Direction(Pin_Direction),
      Pin_Enable(Pin_Enable),
      Pin_Trouble(Pin_Trouble),
      Pin_InPosition(Pin_InPosition),
      Pin_Endstop(Pin_Endstop),
      Pin_Permission(Pin_Permission),
      StepPinNumber(StepPinNumber),
      AxisNomber(AxisNomber)
{

    this->MaxPosition = MaxPositionInMillimeter * StepsPerMillimeter;
    this->maxSpeed = maxSpeed * StepsPerMillimeter;
    this->HomingOffset = HomingOffset;
    this->TimerPeriod = TimerPeriod;
    this->Port = Port;
    this->OutputRegister = OutputRegister;
    this->stepsPerMillimeter = StepsPerMillimeter;
    setAcceleration(acceleration);
    TimerInitialisieren();
    pinMode(Pin_Direction, OUTPUT);
    pinMode(Pin_Enable, OUTPUT);
    pinMode(Pin_InPosition, INPUT_PULLUP);
    pinMode(Pin_Trouble, INPUT_PULLUP);
    pinMode(Pin_Endstop, INPUT_PULLUP);

    digitalWrite(Pin_Direction, LOW);
    digitalWrite(Pin_Enable, LOW);
    uint8_t AktuellerWertPort = *OutputRegister;
    *OutputRegister = AktuellerWertPort | (1 << StepPinNumber);
}

void Axxis::setAcceleration(unsigned long acceleration){
    this->acceleration = acceleration * stepsPerMillimeter;
    this->accelerationPerAccelerationRecalculation = this->acceleration / PROCESSOR_CYCLES_PER_MICROSECOND * ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES / 1000000;
    this->DistanzAbbremsenVonMaxSpeed = (this->maxSpeed * this->maxSpeed) / (2 * this->acceleration);
}

void Axxis::lock()
{
    if (!locked)
    {
        digitalWrite(Pin_Enable, LOW);
        homingAbgeschlossen = false;
        killed = false;
        ErrorID = 0;
    }
}

void Axxis::unlock()
{
    stopAxis();
    locked = true;
    digitalWrite(Pin_Enable, HIGH);
}

bool Axxis::isLocked()
{
    return locked;
}

void Axxis::home()
{
    stopAxis();
    homingAbgeschlossen = false;
    digitalWrite(Pin_Direction, HIGH);
    bool homingAbgeschlossen = false;
    unsigned long myMicros;
    unsigned long microsLastCycle = micros();
    unsigned long mycrosSinceLastExecute = 0;
    const unsigned long intervall = 20 * stepsPerMillimeter;
    bool AchseHatEndstopVerlassen = false;
    while (!homingAbgeschlossen)
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

                if (!digitalReadAverage(Pin_Endstop) & !homingAbgeschlossen)
                {
                    homingAbgeschlossen = true;
                    istPosition = 0;
                }
                if (!homingAbgeschlossen)
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
                // requestHome = true;
            }
        }
    }

    move(HomingOffset * stepsPerMillimeter);

    while (aktiv)
    {
    }
    istPosition = 0;
}

void Axxis::setHomingOffset(unsigned long offset){
    HomingOffset = offset;
    home();
}

int Axxis::getSomeValue()
{
    return currentDirection;
}

void Axxis::printInputStatus()
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

void Axxis::dumpAxisParameter()
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

bool Axxis::digitalReadAverage(int pin)
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

bool Axxis::hasError()
{
    unsigned long int timeStamp;
    const unsigned long int timeTillError = 50;
    if (killed)
    {
        return true;
    }

    if (homingAbgeschlossen & !digitalRead(Pin_Endstop))
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

unsigned int Axxis::getErrorID()
{
    return ErrorID;
}