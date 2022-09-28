#ifndef AXISMOVE_H
#define AXISMOVE_H
#include "Axxis.h"
#include "AxxisTimer.h"
#ifndef SIMULATION
#define SIMULATION false
#endif

void Axxis::move(unsigned long neuePosition)
{
    unsigned long newPosAsSteps = neuePosition * stepsPerMillimeter;
    if (MaxPosition < newPosAsSteps)
    {
        newPosAsSteps = MaxPosition;
    }

    if (SIMULATION)
    {
        Serial.println("move() wird ausgeführt");
        Serial.print("Achse: ");
        Serial.println(AxisNomber);
        Serial.print("newPosAsSteps: ");
        Serial.println(newPosAsSteps / stepsPerMillimeter);
        Serial.print("istPosition: ");
        Serial.println(istPosition / stepsPerMillimeter);
    }
    changeOfDirection = false;
    switch (getMovementType(newPosAsSteps))
    {
    case movementFromZero:
        if (SIMULATION)
        {
            Serial.println("movementType: movementFromZero");
        }
        defaultMove(newPosAsSteps);
        break;
    case movementExtension:
    {
        if (SIMULATION)
        {
            Serial.println("movementType: movementExtension");
        }
        unsigned long speedDifferenceToMaxSpeed = maxSpeed - currentSpeed;
        unsigned long dauerBeschleunigenBisMaxSpeedIn10Millis = speedDifferenceToMaxSpeed * 10000 / acceleration;
        unsigned long distanzBeschleunigen = (speedDifferenceToMaxSpeed * speedDifferenceToMaxSpeed) / (2 * acceleration) + (dauerBeschleunigenBisMaxSpeedIn10Millis * currentSpeed / 10000);
        unsigned long distanzAbbremsen = (maxSpeed * maxSpeed) / (2 * acceleration);
        unsigned long theoretischePositionBeimBeschleunigenAufMaxSpeed;

        if (SIMULATION)
        {
            Serial.print("speedDifferenceToMaxSpeed ");
            Serial.println(speedDifferenceToMaxSpeed / stepsPerMillimeter);
            Serial.print("dauerBeschleunigenBisMaxSpeedIn10Millis ");
            Serial.println(dauerBeschleunigenBisMaxSpeedIn10Millis);
            Serial.print("distanzBeschleunigen ");
            Serial.println(distanzBeschleunigen / stepsPerMillimeter);
            Serial.print("distanzAbbremsen ");
            Serial.println(distanzAbbremsen / stepsPerMillimeter);
        }

        if (currentDirection)
        {
            theoretischePositionBeimBeschleunigenAufMaxSpeed = distanzBeschleunigen + distanzAbbremsen + istPosition;
        }
        else
        {
            if (distanzAbbremsen + istPosition > istPosition)
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = 0;
            }
            else
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = istPosition - distanzBeschleunigen - distanzAbbremsen;
            }
        }

        if (SIMULATION)
        {
            Serial.print("theoretischePositionBeimBeschleunigenAufMaxSpeed ");
            Serial.println(theoretischePositionBeimBeschleunigenAufMaxSpeed / stepsPerMillimeter);
        }

        bool esWirdAufMaxSpeedBeschleunigt;
        if (currentDirection)
        {
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed < newPosAsSteps)
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
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed > newPosAsSteps)
            {
                esWirdAufMaxSpeedBeschleunigt = true;
            }
            else
            {
                esWirdAufMaxSpeedBeschleunigt = false;
            }
        }

        if (SIMULATION)
        {
            if (esWirdAufMaxSpeedBeschleunigt)
            {
                Serial.println("Es wird auf Max Speed beschleunigt");
            }
            else
            {
                Serial.println("Es wird nicht auf Max Speed beschleunigt");
            }
        }

        if (esWirdAufMaxSpeedBeschleunigt)
        {
            defaultMove(newPosAsSteps);
        }
        else
        {
            unsigned long Abbremsdistanz = (currentSpeed * currentSpeed) / (2 * acceleration);
            unsigned long PositionBeimDirektenAbbremsen;

            if (currentDirection)
            {
                PositionBeimDirektenAbbremsen = istPosition + Abbremsdistanz;
            }
            else
            {
                PositionBeimDirektenAbbremsen = istPosition - Abbremsdistanz;
            }
            if (SIMULATION)
            {
                Serial.print("PositionBeimDirektenAbbremsen: ");
                Serial.println(PositionBeimDirektenAbbremsen / stepsPerMillimeter);
            }

            unsigned long DifferenzZurSollPositionWennDirektAbgebremstWird;
            if (currentDirection)
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = newPosAsSteps - PositionBeimDirektenAbbremsen;
            }
            else
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = PositionBeimDirektenAbbremsen - newPosAsSteps;
            }

            unsigned long HaelfteDerDifferenz = DifferenzZurSollPositionWennDirektAbgebremstWird / 2;

            if (currentDirection)
            {
                posStartDeccelerating = newPosAsSteps - Abbremsdistanz - HaelfteDerDifferenz;
            }
            else
            {
                posStartDeccelerating = newPosAsSteps + Abbremsdistanz + HaelfteDerDifferenz;
            }
            if (SIMULATION)
            {
                Serial.print("posStartDeccelerating: ");
                Serial.println(posStartDeccelerating / stepsPerMillimeter);
            }
            digitalWrite(Pin_Direction, currentDirection);
            accelerating = true;
            deccelerating = false;
            CyclesSinceLastAccelerationCalculation = 0;
            sollPosition = newPosAsSteps;
            startAxis();
        }

        break;
    }
    case movementWithChangeOfDirection:
    {
        if (SIMULATION)
        {
            Serial.println("movementType: movementWithChangeOfDirection");
        }
        sollPosition = getStopPosition();
        if (SIMULATION)
        {
            Serial.print("stopp Position: ");
            Serial.println(sollPosition / stepsPerMillimeter);
        }
        accelerating = false;
        deccelerating = true;
        changeOfDirection = true;
        sollPositionNachRichtungswechsel = newPosAsSteps;

        // if (!AxisL -> aktiv){
        //   startAxis(AxisNomber);
        //}

        if (SIMULATION)
        {
            Serial.print("sollPositionNachRichtungswechsel: ");
            Serial.println(newPosAsSteps / stepsPerMillimeter);
        }

        break;
    }
    default:
    {
        if (SIMULATION)
        {
            Serial.println("movementType: garnichts????");
        }
        break;
    }
    }

    // interrupts();
    if (SIMULATION)
    {
        // Serial.println("Interrupts wurden gestartet");
    }
}


unsigned long Axxis::getStopPosition()
{
    if (currentDirection)
    {
        return istPosition + ((currentSpeed * currentSpeed) / (2 * acceleration));
    }
    else
    {
        return istPosition - ((currentSpeed * currentSpeed) / (2 * acceleration));
    }
}

bool Axxis::stoppositionLiegtHinterSollposition(unsigned long newPosAsSteps)
{
    unsigned long stopPosition = getStopPosition();
    if (currentDirection)
    {
        return (stopPosition > newPosAsSteps);
    }
    else
    {
        return (stopPosition < newPosAsSteps);
    }
}

enum movementType Axxis::getMovementType(unsigned long newPosAsSteps)
{
    if (aktiv & ((currentDirection & (istPosition > newPosAsSteps)) || (!currentDirection & (istPosition < newPosAsSteps))))
    {
        return movementWithChangeOfDirection;
    }
    else if (aktiv & ((currentDirection & (istPosition < newPosAsSteps)) || (!currentDirection & (istPosition > newPosAsSteps))))
    {
        if (stoppositionLiegtHinterSollposition(newPosAsSteps))
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
// defaultMove -> Bewegungsablauf von null oder in gleicher richtung wenn maximale geschwindigkeit erreicht wird. Der Motor wird gestartet und "posStartDeccelerating"
// wird auf die Abbremsdistanz von Max Speed gesetzt
void Axxis::defaultMove(unsigned long newPosAsSteps)
{
    if (SIMULATION)
    {
        Serial.println("defaultMove() wird ausgeführt");
    }
    if (istPosition != newPosAsSteps)
    {
        unsigned long distanzAbbremsen = DistanzAbbremsenVonMaxSpeed;
        if (SIMULATION)
        {
            Serial.print("distanzAbbremsen ->");
            Serial.println(distanzAbbremsen / stepsPerMillimeter);
        }
        currentDirection = newPosAsSteps > istPosition;
        unsigned long distanz;
        if (currentDirection)
        {
            distanz = newPosAsSteps - istPosition;
        }
        else
        {
            distanz = istPosition - newPosAsSteps;
        }
        if (distanzAbbremsen > distanz / 2)
        {
            distanzAbbremsen = distanz / 2;
            if (SIMULATION)
            {
                Serial.println("not reaching full speed -> recalculating");
                Serial.print("distanzAbbremsen ->");
                Serial.println(distanzAbbremsen / stepsPerMillimeter);
            }
        }
        if (currentDirection)
        {
            posStartDeccelerating = newPosAsSteps - distanzAbbremsen;
        }
        else
        {
            posStartDeccelerating = newPosAsSteps + distanzAbbremsen;
        }
        digitalWrite(Pin_Direction, currentDirection);
        if (SIMULATION)
        {
            Serial.print("Pin Direction: ");
            Serial.println(Pin_Direction);
            if (currentDirection)
            {
                Serial.println("Drehrichtung Forwährts");
            }
            else
            {
                Serial.println("Drehrichtung Rückwährts");
            }
        }

        accelerating = true;
        deccelerating = false;
        aktiv = true;
        //millisLastCycle = millis();
        CyclesSinceLastAccelerationCalculation = 0;
        sollPosition = newPosAsSteps;
        startAxis();
        if (SIMULATION)
        {
            Serial.print("posStartDeccelerating ->");
            Serial.println(posStartDeccelerating / stepsPerMillimeter);
        }
    }
}

#endif