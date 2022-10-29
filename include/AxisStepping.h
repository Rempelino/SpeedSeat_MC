#include "Axis.h"

ISR(TIMER3_COMPA_vect)
{
    Axis::durationSinceLastInterrupt = Axis::TimerPeriod;
    Axis::TimerPeriod = 0xFFFF;

    for (unsigned i = 0; i < Axis::AxisCounter; i++)
    {
        (*Axis::ExecutePointer[i])();
    }

    if (Axis::analyzeWorkload)
    {
        Axis::workload = (float)(TCNT3) / (float)(OCR3A);
        Axis::analyzeWorkload = false;
    }
    OCR3A = Axis::TimerPeriod;
}

void Axis::execute()
{
    if (homingActive)
    {
        executeHoming();
    }
    if (!active)
    {
        return;
    }
    unsigned x = durationSinceLastInterrupt + TCNT3;

    if (timeTillNextExecute > x + TCNT3 && durationSinceLastInterrupt < x)
    {
        timeTillNextExecute -= durationSinceLastInterrupt;
    }
    else
    {
        timeTillNextExecute = timeTillNextExecute + table[tablePosition] - durationSinceLastInterrupt;
        toggle = !toggle;
        if (toggle)
        {
            *step_Port &= ~step_Bit;
        }
        else
        {
            *step_Port |= step_Bit;
        }

        // count current position
        if (direction)
        {
            if (istPosition < maxPosition || !softwareLimitsEnabled)
            {
                istPosition++;
            }
            else
            {
                commandFinished();
            }
        }
        else
        {
            if (istPosition > 0 || !softwareLimitsEnabled)
            {
                istPosition--;
            }
            else
            {
                commandFinished();
            }
        }

        // check for start of decceleration
        if ((direction && istPosition >= positionStartDecelerating) || (!direction && istPosition <= positionStartDecelerating))
        {
            if (!movingVelocity)
            {
                decelerating = true;
            }
        }

        // check for finished command
        if (focusOnTracking && !movingVelocity && !stopping)
        {
            if ((direction && istPosition >= sollPosition) || (!direction && istPosition <= sollPosition))
            {
                commandFinished();
            }
        }
    }

    while (timeTillNextSpeedUpdate <= durationSinceLastInterrupt)
    {
        timeTillNextSpeedUpdate += accelerationRecalculationPeriod;
        if (decelerating)
        {
            if (tablePosition > 0)
            {
                tablePosition--;
            }
            else if (focusOnSmootheness || stopping)
            {
                commandFinished();
            }
        }
        else
        {
            if (tablePosition < maximumTablePosition)
            {
                tablePosition++;
            }
        }
        requiredTimerPeriod = table[tablePosition];
    }
    timeTillNextSpeedUpdate -= durationSinceLastInterrupt;

    if (TimerPeriod > requiredTimerPeriod)
    {
        TimerPeriod = requiredTimerPeriod;
    }
}

void Axis::commandFinished()
{
    active = false;
    stopping = false;
    tablePosition = 0;
    requiredTimerPeriod = 0xFFFF;

    if (changeOfDirection)
    {
        moveAbsoluteInternal(sollPositionNachRichtungswechsel);
    }
    else
    {
        resetSpeed();
        resetAcceleration();
    }
}

void Axis::disableStepping()
{
    if (steppingIsEnabled)
    {
        steppingIsEnabled = false;
        TIMSK3 = TIMSK3 & ~(1 << OCIE1A);
        OCR3A = 65535;
    }
}

void Axis::enableStepping()
{
    if (!steppingIsEnabled)
    {
        steppingIsEnabled = true;
        TIMSK3 |= (1 << OCIE1A);
    }
}
