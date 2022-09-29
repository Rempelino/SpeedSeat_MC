#ifndef AXISSTEPS_H
#define AXISSTEPS_H
#ifndef CALCULATE_ACCELERATION_VIA_INTERRUPT
#define CALCULATE_ACCELERATION_VIA_INTERRUPT false
#endif
#include "Axxis.h"
const unsigned int speedAt65535ProzessorCyclesPerStep = 1000000 / (65535 / PROCESSOR_CYCLES_PER_MICROSECOND) + 1;

void Axxis::newStep()
{
  // Den Pin HIGH oder LOW schalten
  if (!CALCULATE_ACCELERATION_VIA_INTERRUPT)
  {
    CyclesSinceLastAccelerationCalculation = CyclesSinceLastAccelerationCalculation + stepPeriodInProcessorCycles;
    while (CyclesSinceLastAccelerationCalculation >= ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES)
    {
      CyclesSinceLastAccelerationCalculation = CyclesSinceLastAccelerationCalculation - ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES;
      stepPeriodInProcessorCycles = getTimeTillNextStep();
      *TimerPeriod = stepPeriodInProcessorCycles;
    }
  }
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

  // Die enstprechende Position High oder Low schalten
  if (currentDirection)
  {
    istPosition++;
  }
  else
  {
    if (istPosition == 0)
    {
      stopAxis();
      killed = true;
      ErrorID = 1;
    }
    else
    {
      istPosition--;
    }
  }

  if (istPosition == MaxPosition + 1)
  {
    stopAxis();
    killed = true;
    ErrorID = 2;
  }

  if (istPosition == posStartDeccelerating)
  {
    deccelerating = true;
    accelerating = false;
  }
  if ((istPosition == sollPosition) || (runningMinSpeed & changeOfDirection) || (runningMinSpeed & deccelerating) || (istPosition == MaxPosition) || (istPosition == 0))
  {
    deccelerating = false;
    accelerating = false;
    aktiv = false;
    currentSpeed = 0;
    stepPeriodInProcessorCycles = 65535;
    ;
    stopAxis();
    if (changeOfDirection)
    {
      move(sollPositionNachRichtungswechsel);
    }
  }
}

uint16_t Axxis::getTimeTillNextStep()
{
  bool minSpeed = false;
  if (accelerating)
  {
    currentSpeed = currentSpeed + accelerationPerAccelerationRecalculation;
    if (currentSpeed > maxSpeed)
    {
      currentSpeed = maxSpeed;
      accelerating = false;
    }
  }

  if (deccelerating)
  {
    if (currentSpeed >= accelerationPerAccelerationRecalculation)
    {
      currentSpeed = currentSpeed - accelerationPerAccelerationRecalculation;
    }
    else
    {
      currentSpeed = 0;
    }
  }
  if (currentSpeed < speedAt65535ProzessorCyclesPerStep)
  {
    currentSpeed = speedAt65535ProzessorCyclesPerStep;
    minSpeed = true;
  }
  runningMinSpeed = minSpeed;
  return 1000000 / currentSpeed * 16;
}

void Axxis::recalculateAccelleration()
{
  if (aktiv)
  {
    stepPeriodInProcessorCycles = getTimeTillNextStep();
    *TimerPeriod = stepPeriodInProcessorCycles;
  }
}

#endif