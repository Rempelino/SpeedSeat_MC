#include "Axis.h"
void Axis::executeHoming()
{
    switch (homingStep)
    {
    case waitForAxisToStop:
        if (active)
        {
            stop();
        }
        else
        {
            disableSoftwareLimits();
            setTempAcceleration(accelerationWhileHoming);
            homingStep = clearEndstop;
        }
        break;

    case clearEndstop:
        if (!digitalRead(Pin_Endstop))
        {
            if (!active)
            {
                moveVelocityInternal(speedWhileHoming, true);
            }
        }
        else
        {
            stop();
            homingStep = driveToEndstop;
        }
        break;

    case driveToEndstop:
        if (!stopping)
        {
            if (!digitalRead(Pin_Endstop))
            {
                stop();
                homingStep = moveFromEndstopAfterHoming;
            }
            else
            {
                if (!active)
                {
                    setTempAcceleration(accelerationWhileHoming);
                    moveVelocityInternal(speedWhileHoming, false);
                }
            }
        }
        break;

    case moveFromEndstopAfterHoming:
        if (!stopping && !active)
        {
            setTempAcceleration(accelerationWhileHoming);
            setTempSpeed(speedWhileHoming);
            istPosition = 0;
            sollPosition = 0;
            moveAbsoluteInternal(homingOffset);
            homingStep = waitingForEndOfMovement;
        }
        break;

    case waitingForEndOfMovement:
        if (!active)
        {
            resetAcceleration();
            resetSpeed();
            enableSoftwareLimits();
            istPosition = 0;
            AxisIsHomed = true;
            homingActive = false;
        }
        break;

    default:
        break;
    }
}