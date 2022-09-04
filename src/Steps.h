const unsigned int speedAt65535ProzessorCyclesPerStep = 1000000/(65535/PROCESSOR_CYCLES_PER_MICROSECOND)+1;

uint16_t getTimeTillNextStep(){
  bool minSpeed = false;
  if (Axis->accelerating){
    Axis ->currentSpeed = Axis->currentSpeed  + Axis->accelerationPerAccelerationRecalculation;
    if (Axis->currentSpeed > Axis->maxSpeed){
      Axis->currentSpeed = Axis->maxSpeed;
      Axis->accelerating = false;
    }
  }

  if (Axis->deccelerating){
    if (Axis ->currentSpeed >= Axis->accelerationPerAccelerationRecalculation){
      Axis ->currentSpeed = Axis->currentSpeed  - Axis->accelerationPerAccelerationRecalculation;
    }else{
      Axis ->currentSpeed = 0;
    }
  }
  //HENRY DU MUSST HIER NOCHMAL SCHAUEN OB MAN DEN RUNDUNGSFEHLER RAUSSUCHEN MUSS
  if (Axis ->currentSpeed < speedAt65535ProzessorCyclesPerStep){
      Axis ->currentSpeed = speedAt65535ProzessorCyclesPerStep;
      minSpeed = true;
  }
  Axis -> runningMinSpeed = minSpeed;
   return 1000000 / Axis -> currentSpeed * 16;

}

void newStep(){
  //Den Pin HIGH oder LOW schalten
  if(!CALCULATE_ACCELERATION_VIA_INTERRUPT){
    Axis->CyclesSinceLastAccelerationCalculation = Axis->CyclesSinceLastAccelerationCalculation + Axis->stepPeriodInProcessorCycles;
    while (Axis ->CyclesSinceLastAccelerationCalculation >= ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES){
        Axis->CyclesSinceLastAccelerationCalculation = Axis->CyclesSinceLastAccelerationCalculation - ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES;
        Axis->stepPeriodInProcessorCycles = getTimeTillNextStep();
        *Axis->TimerPeriod = Axis->stepPeriodInProcessorCycles;
    }
  }
  uint8_t AktuellerWertPort = *Axis->Port;
  if (Axis->toggle){
      *Axis->Port = AktuellerWertPort |(1<<Axis->StepPinNumber);
       Axis->toggle = false;
  }else{
      *Axis->Port = AktuellerWertPort &~(1<<Axis->StepPinNumber);
      Axis->toggle = true;
  }

//Die enstprechende Position High oder Low schalten
  if (Axis->currentDirection){
    Axis->istPosition++;
  }else{
    if (Axis->istPosition == 0){
      ErrorNumber = Axis->AxisNomber * 10 + 1;
      kill();
    }else{
    Axis->istPosition--;
    }
  }

  if (Axis->istPosition == Axis->MaxPosition+1){
    ErrorNumber = Axis->AxisNomber * 10 + 2;
    kill();
  }

  if (Axis->istPosition == Axis->posStartDeccelerating){
    Axis->deccelerating = true;
    Axis->accelerating = false;
  }
  if ((Axis->istPosition == Axis->sollPosition)||(Axis->runningMinSpeed & Axis->changeOfDirection)||(Axis->runningMinSpeed & Axis->deccelerating)||(Axis->istPosition == Axis->MaxPosition)||(Axis->istPosition == 0)){
    Axis->deccelerating = false;
    Axis->accelerating = false;
    Axis->aktiv = false;
    Axis->currentSpeed = 0;
    Axis->stepPeriodInProcessorCycles = 65535;;
    stopAxis(Axis->AxisNomber);
    if (Axis->changeOfDirection){
      move(Axis->AxisNomber,Axis->sollPositionNachRichtungswechsel);
    }
  } 
}


void recalculateAccelleration(){
  int x = 0;
  while (x<3){
    Axis = getAxis(x);
      if (Axis->aktiv){
        Axis->stepPeriodInProcessorCycles = getTimeTillNextStep();
        *Axis->TimerPeriod = Axis->stepPeriodInProcessorCycles;
      }
    x++;
  }
}



