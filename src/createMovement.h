struct Achse *AxisL;

unsigned long int getStopPosition()
{
    if (AxisL->currentDirection)
    {
        return AxisL->istPosition + ((AxisL->currentSpeed * AxisL->currentSpeed) / (2 * AxisL->acceleration));
    }
    else
    {
        return AxisL->istPosition - ((AxisL->currentSpeed * AxisL->currentSpeed) / (2 * AxisL->acceleration));
    }
}

bool stoppositionLiegtHinterSollposition(unsigned long int neuePosition)
{
    unsigned long int stopPosition = getStopPosition();
    if (AxisL->currentDirection)
    {
        return (stopPosition > neuePosition);
    }
    else
    {
        return (stopPosition < neuePosition);
    }
}

enum movementType getMovementType(unsigned long int neuePosition)
{
    if (AxisL->aktiv & ((AxisL->currentDirection & (AxisL->istPosition > neuePosition)) || (!AxisL->currentDirection & (AxisL->istPosition < neuePosition))))
    {
        return movementWithChangeOfDirection;
    }
    else if (AxisL->aktiv & ((AxisL->currentDirection & (AxisL->istPosition < neuePosition)) || (!AxisL->currentDirection & (AxisL->istPosition > neuePosition))))
    {
        if (stoppositionLiegtHinterSollposition(neuePosition))
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
void defaultMove(unsigned long int neuePosition)
{
    if (SIMULATION)
    {
        Serial.println("defaultMove() wird ausgeführt");
    }
    if (AxisL->istPosition != neuePosition)
    {
        unsigned long int distanzAbbremsen = AxisL->DistanzAbbremsenVonMaxSpeed;
        if (SIMULATION)
        {
            Serial.print("distanzAbbremsen ->");
            Serial.println(distanzAbbremsen / STEPS_PER_MM);
        }
        AxisL->currentDirection = neuePosition > AxisL->istPosition;
        unsigned long int distanz;
        if (AxisL->currentDirection)
        {
            distanz = neuePosition - AxisL->istPosition;
        }
        else
        {
            distanz = AxisL->istPosition - neuePosition;
        }
        if (distanzAbbremsen > distanz / 2)
        {
            distanzAbbremsen = distanz / 2;
            if (SIMULATION)
            {
                Serial.println("not reaching full speed -> recalculating");
                Serial.print("distanzAbbremsen ->");
                Serial.println(distanzAbbremsen / STEPS_PER_MM);
            }
        }
        if (AxisL->currentDirection)
        {
            AxisL->posStartDeccelerating = neuePosition - distanzAbbremsen;
        }
        else
        {
            AxisL->posStartDeccelerating = neuePosition + distanzAbbremsen;
        }
        digitalWrite(AxisL->Pin.Direction, AxisL->currentDirection);
        if (SIMULATION)
        {
            Serial.print("Pin Direction: ");
            Serial.println(AxisL->Pin.Direction);
            if (AxisL->currentDirection)
            {
                Serial.println("Drehrichtung Forwährts");
            }
            else
            {
                Serial.println("Drehrichtung Rückwährts");
            }
        }

        AxisL->accelerating = true;
        AxisL->deccelerating = false;
        AxisL->aktiv = true;
        millisLastCycle = millis();
        AxisL->CyclesSinceLastAccelerationCalculation = 0;
        AxisL->sollPosition = neuePosition;
        startAxis(AxisL->AxisNomber);
        if (SIMULATION)
        {
            Serial.print("posStartDeccelerating ->");
            Serial.println(AxisL->posStartDeccelerating / STEPS_PER_MM);
        }
    }
}
void move(unsigned short int AxisNomber, unsigned long int neuePosition)
{    AxisL = getAxis(AxisNomber);
    if (AxisL->MaxPosition < neuePosition){
        neuePosition = AxisL->MaxPosition;
    }
    if (SIMULATION)
    {
        Serial.println("move() wird ausgeführt");
        Serial.print("Achse: ");
        Serial.println(AxisL->AxisNomber);
        Serial.print("neuePosition: ");
        Serial.println(neuePosition / STEPS_PER_MM);
        Serial.print("istPosition: ");
        Serial.println(AxisL->istPosition / STEPS_PER_MM);
    }
    // noInterrupts();
    //stopAxis(AxisNomber);
    AxisL->changeOfDirection = false;
    switch (getMovementType(neuePosition))
    {
    case movementFromZero:
        if (SIMULATION)
        {
            Serial.println("movementType: movementFromZero");
        }
        defaultMove(neuePosition);
        break;
    case movementExtension:
    {
        if (SIMULATION)
        {
            Serial.println("movementType: movementExtension");
        }
        unsigned long int speedDifferenceToMaxSpeed = AxisL->maxSpeed - AxisL->currentSpeed;
        unsigned long int dauerBeschleunigenBisMaxSpeedIn10Millis = speedDifferenceToMaxSpeed * 10000 / AxisL->acceleration;
        unsigned long int distanzBeschleunigen = (speedDifferenceToMaxSpeed * speedDifferenceToMaxSpeed) / (2 * AxisL->acceleration) + (dauerBeschleunigenBisMaxSpeedIn10Millis * AxisL->currentSpeed / 10000);
        unsigned long int distanzAbbremsen = (AxisL->maxSpeed * AxisL->maxSpeed) / (2 * AxisL->acceleration);
        unsigned long int theoretischePositionBeimBeschleunigenAufMaxSpeed;

        if (SIMULATION)
        {
            Serial.print("speedDifferenceToMaxSpeed ");
            Serial.println(speedDifferenceToMaxSpeed / STEPS_PER_MM);
            Serial.print("dauerBeschleunigenBisMaxSpeedIn10Millis ");
            Serial.println(dauerBeschleunigenBisMaxSpeedIn10Millis);
            Serial.print("distanzBeschleunigen ");
            Serial.println(distanzBeschleunigen / STEPS_PER_MM);
            Serial.print("distanzAbbremsen ");
            Serial.println(distanzAbbremsen / STEPS_PER_MM);
        }

        if (AxisL->currentDirection)
        {
            theoretischePositionBeimBeschleunigenAufMaxSpeed = distanzBeschleunigen + distanzAbbremsen + AxisL->istPosition;
        }
        else
        {
            if (distanzAbbremsen + AxisL->istPosition > AxisL->istPosition)
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = 0;
            }
            else
            {
                theoretischePositionBeimBeschleunigenAufMaxSpeed = AxisL->istPosition - distanzBeschleunigen - distanzAbbremsen;
            }
        }

        if (SIMULATION)
        {
            Serial.print("theoretischePositionBeimBeschleunigenAufMaxSpeed ");
            Serial.println(theoretischePositionBeimBeschleunigenAufMaxSpeed / STEPS_PER_MM);
        }

        bool esWirdAufMaxSpeedBeschleunigt;
        if (AxisL->currentDirection)
        {
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed < neuePosition)
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
            if (theoretischePositionBeimBeschleunigenAufMaxSpeed > neuePosition)
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
            defaultMove(neuePosition);
        }
        else
        {
            unsigned long int Abbremsdistanz = (AxisL->currentSpeed * AxisL->currentSpeed) / (2 * AxisL->acceleration);
            unsigned long int PositionBeimDirektenAbbremsen;

            if (AxisL->currentDirection)
            {
                PositionBeimDirektenAbbremsen = AxisL->istPosition + Abbremsdistanz;
            }
            else
            {
                PositionBeimDirektenAbbremsen = AxisL->istPosition - Abbremsdistanz;
            }
            if (SIMULATION)
            {
                Serial.print("PositionBeimDirektenAbbremsen: ");
                Serial.println(PositionBeimDirektenAbbremsen / STEPS_PER_MM);
            }

            unsigned long int DifferenzZurSollPositionWennDirektAbgebremstWird;
            if (AxisL->currentDirection)
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = neuePosition - PositionBeimDirektenAbbremsen;
            }
            else
            {
                DifferenzZurSollPositionWennDirektAbgebremstWird = PositionBeimDirektenAbbremsen - neuePosition;
            }

            unsigned long int HaelfteDerDifferenz = DifferenzZurSollPositionWennDirektAbgebremstWird / 2;

            if (AxisL->currentDirection)
            {
                AxisL->posStartDeccelerating = neuePosition - Abbremsdistanz - HaelfteDerDifferenz;
            }
            else
            {
                AxisL->posStartDeccelerating = neuePosition + Abbremsdistanz + HaelfteDerDifferenz;
            }
            if (SIMULATION)
            {
                Serial.print("posStartDeccelerating: ");
                Serial.println(AxisL->posStartDeccelerating / STEPS_PER_MM);
            }
            digitalWrite(AxisL->Pin.Direction, AxisL->currentDirection);
            AxisL->accelerating = true;
            AxisL->deccelerating = false;
            AxisL->CyclesSinceLastAccelerationCalculation = 0;
            AxisL->sollPosition = neuePosition;
            startAxis(AxisL->AxisNomber);
        }

        break;
    }
    case movementWithChangeOfDirection:
    {
        if (SIMULATION)
        {
            Serial.println("movementType: movementWithChangeOfDirection");
        }
        AxisL->sollPosition = getStopPosition();
        if (SIMULATION)
        {
            Serial.print("stopp Position: ");
            Serial.println(AxisL->sollPosition / STEPS_PER_MM);
        }
        AxisL->accelerating = false;
        AxisL->deccelerating = true;
        AxisL->changeOfDirection = true;
        AxisL->sollPositionNachRichtungswechsel = neuePosition;

        //if (!AxisL -> aktiv){
          //  startAxis(AxisNomber);
        //}

        if (SIMULATION)
        {
            Serial.print("sollPositionNachRichtungswechsel: ");
            Serial.println(neuePosition / STEPS_PER_MM);
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

void makeCustomMove()
{
    if (SIMULATION)
    {
        Serial.println("Hier beginnt der custom move -------------------------------------------------");
    }
    
    move(0,getSteps(100));
    while(X_Axis.aktiv){}
    move(0,0);

    if (SIMULATION)
    {
        Serial.println("Hier endet der custom move -------------------------------------------------");
    }
}