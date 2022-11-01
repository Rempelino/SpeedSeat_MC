#include <Arduino.h>
#include "Axis.h"

// Public Functions
void Axis::moveAbsolute(unsigned long newPosition)
{
    if (AxisIsHomed)
    {
        moveAbsoluteInternal(newPosition * stepsPerMillimeter);
    }
}

void Axis::moveAbsoluteSteps(unsigned long newPosition)
{
    if (AxisIsHomed)
    {
        moveAbsoluteInternal(newPosition);
    }
}

void Axis::moveRelative(unsigned long newPosition, bool direction)
{
    if (AxisIsHomed)
    {

        moveRelativeInternal(newPosition * stepsPerMillimeter, direction);
    }
}

void Axis::moveVelocity(unsigned long speed, bool direction)
{
    if (AxisIsHomed)
    {

        moveVelocityInternal(speed * stepsPerMillimeter, direction);
    }
}

unsigned long speed;
void Axis::moveAbsoluteInternal(unsigned long newPosition)
{
    if(!AxisIsReady()){
        return;
    }

    if (newPosition > maxPosition)
    {
        newPosition = maxPosition;
    }

    lastCommandPosition = newPosition;
    speed = getSpeed();
    changeOfDirection = false;
    switch (getMovementType(newPosition))
    {
    case movementFromZero:
        if (!active)
        {
            tablePosition = 0;
        }
        defaultMove(newPosition);
        break;
    case movementExtension:
    {
        unsigned long speedDifferenceToMaxSpeed = maxSpeed - speed;
        unsigned long dauerBeschleunigenBisMaxSpeedIn10Millis = speedDifferenceToMaxSpeed * 10000 / acceleration;
        unsigned long distanzBeschleunigen = (speedDifferenceToMaxSpeed * speedDifferenceToMaxSpeed) / (2 * acceleration) + (dauerBeschleunigenBisMaxSpeedIn10Millis * speed / 10000);
        unsigned long distanzAbbremsen = (maxSpeed * maxSpeed) / (2 * acceleration);
        unsigned long theoretischePositionBeimBeschleunigenAufMaxSpeed;

        if (direction)
        {
            theoretischePositionBeimBeschleunigenAufMaxSpeed = distanzBeschleunigen + distanzAbbremsen + istPosition;
        }
        else
        {
            if (distanzBeschleunigen + distanzAbbremsen > istPosition)
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = 0;
            }
            else
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = istPosition - distanzBeschleunigen - distanzAbbremsen;
            }
        }

        bool esWirdAufMaxSpeedBeschleunigt;
        if (isRunningMaxSpeed())
        {
            esWirdAufMaxSpeedBeschleunigt = true;
        }
        else if (direction)
        {
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed < newPosition)
            {
                esWirdAufMaxSpeedBeschleunigt = true;
            }
            else
            {
                esWirdAufMaxSpeedBeschleunigt = false;
            }
        }
        else
        {
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed > newPosition)
            {
                esWirdAufMaxSpeedBeschleunigt = true;
            }
            else
            {
                esWirdAufMaxSpeedBeschleunigt = false;
            }
        }

        if (esWirdAufMaxSpeedBeschleunigt)
        {
            defaultMove(newPosition);
        }
        else
        {
            unsigned long Abbremsdistanz = (speed * speed) / (2 * acceleration);
            unsigned long PositionBeimDirektenAbbremsen = getStopPosition();

            unsigned long DifferenzZurSollPositionWennDirektAbgebremstWird;
            if (direction)
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = newPosition - PositionBeimDirektenAbbremsen;
            }
            else
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = PositionBeimDirektenAbbremsen - newPosition;
            }

            unsigned long HaelfteDerDifferenz = DifferenzZurSollPositionWennDirektAbgebremstWird / 2;
            if (direction)
            {
                positionStartDecelerating = newPosition - Abbremsdistanz - HaelfteDerDifferenz;
            }
            else
            {
                positionStartDecelerating = newPosition + Abbremsdistanz + HaelfteDerDifferenz;
            }

            if (positionStartDecelerating > 300 * 40)
            {
                Serial.println();
                Serial.println(positionStartDecelerating);
                Serial.println();
                while (1)
                {
                }
            }
            digitalWrite(Pin_Direction, direction);
            decelerating = false;
            sollPosition = newPosition;
            active = true;
        }
        break;
    }
    case movementWithChangeOfDirection:
    {
        sollPosition = getStopPosition();
        decelerating = true;
        changeOfDirection = true;
        sollPositionNachRichtungswechsel = newPosition;
        break;
    }
    default:
    {
        break;
    }
    }
}

void Axis::moveRelativeInternal(unsigned long newPosition, bool direction)
{
    if(!AxisIsReady()){
        return;
    }

    if (direction)
    {
        moveAbsoluteInternal(istPosition + newPosition);
    }
    else
    {
        if (newPosition < istPosition || !softwareLimitsEnabled)
        {
            moveAbsoluteInternal(istPosition - newPosition);
        }
        else
        {
            moveAbsoluteInternal(0);
        }
    }
}

void Axis::moveVelocityInternal(unsigned long speed, bool direction)
{
    if(!AxisIsReady()){
        return;
    }

    unsigned currentTimerPeriod;
    if (active)
    {
        currentTimerPeriod = table[tablePosition];
    }

    setTempSpeed(speed);
    if (active)
    {
        unsigned i;
        for (i = 0; i < tableSize; i++)
        {
            if (table[i] < currentTimerPeriod)
            {
                break;
            }
            tablePosition = i;
        }
    }
    this->direction = direction;
    movingVelocity = true;
    decelerating = false;
    digitalWrite(Pin_Direction, direction);
    active = true;
}

void Axis::stop()
{
    if (active)
    {
        movingVelocity = false;
        stopping = true;
        decelerating = true;
    }
}

// defaultMove -> Bewegungsablauf von null oder in gleicher richtung wenn maximale geschwindigkeit erreicht wird. Der Motor wird gestartet und "positionStartDecelerating"
// wird auf die Abbremsdistanz von Max Speed gesetzt
void Axis::defaultMove(unsigned long newPosition)
{
    if (newPosition == sollPosition)
    {
        return;
    }
    unsigned long breakingDistance = calculateBreakingDistance();
    direction = newPosition > istPosition;
    unsigned long distanz;
    if (direction)
    {
        distanz = newPosition - istPosition;
    }
    else
    {
        distanz = istPosition - newPosition;
    }

    if (breakingDistance > distanz / 2)
    {
        if (!active)
        {
            breakingDistance = distanz / 2;
        }
        else
        {
            unsigned long breakingDistanceFromCurrentSpeed = (speed * speed) / (2 * acceleration);
            double distanceAccellerating = (double)((distanz - breakingDistanceFromCurrentSpeed) / 2);
            // v = a * SQRT((2*s)/a)
            unsigned long speedDifferenceFromAccelerating = (unsigned long)((double)(acceleration)*sqrt(2 * distanceAccellerating / (double)(acceleration)));
            if (speedDifferenceFromAccelerating + speed < maxSpeed)
            {
                breakingDistance = breakingDistanceFromCurrentSpeed + (unsigned long)(distanceAccellerating);
            }
        }
    }
    if (direction)
    {
        positionStartDecelerating = newPosition - breakingDistance;
    }
    else
    {
        positionStartDecelerating = newPosition + breakingDistance;
    }

    digitalWrite(Pin_Direction, direction);
    decelerating = false;
    sollPosition = newPosition;
    active = true;
}

enum Axis::movementType Axis::getMovementType(unsigned long newPosition)
{
    if (active & ((direction & (istPosition > newPosition)) || (!direction & (istPosition < newPosition))))
    {
        return movementWithChangeOfDirection;
    }
    else if (active & ((direction & (istPosition < newPosition)) || (!direction & (istPosition > newPosition))))
    {
        if (stoppositionLiegtHinterSollposition(newPosition))
        {
            return movementWithChangeOfDirection;
        }
        else
        {
            return movementExtension;
        }
    }
    else
    {
        return movementFromZero;
    }
}

bool Axis::stoppositionLiegtHinterSollposition(unsigned long newPosition)
{
    unsigned long stopPosition = getStopPosition();
    if (direction)
    {
        return (stopPosition > newPosition);
    }
    else
    {
        return (stopPosition < newPosition);
    }
}

unsigned long Axis::getStopPosition()
{
    unsigned long speed = getSpeed();
    unsigned long pos;
    if (direction)
    {
        return istPosition + ((speed * speed) / (2 * acceleration));
    }
    else
    {
        pos = istPosition - ((speed * speed) / (2 * acceleration));
        if (pos > istPosition)
        {
            return 0;
        }
        else
        {
            return pos;
        }
    }
}
