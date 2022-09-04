// 3 Timer initialisieren für 3 Motoren (Timer 3,4,5 für Achse 0,1,2)


void stopAxis(short unsigned int Axis){
  if(SIMULATION){
    Serial.print("Stopping Axis: ");Serial.println(Axis);
  }

  if (Axis == 0){
    TIMSK3 = TIMSK3 & ~(1 << OCIE1A);
    OCR3A = 65535;
  }else if (Axis == 1){
    TIMSK4 = TIMSK4 & ~(1 << OCIE1A);
    OCR4A = 65535;
  }else if (Axis == 2){
    TIMSK5 = TIMSK5 & ~(1 << OCIE1A);
    OCR5A = 65535;
  }
}

void startAxis(short unsigned int Axis){
  if(SIMULATION){
    Serial.print("Axis: ");Serial.print(Axis);Serial.println(" wird gestartet.");
  }

  if (Axis == 0){
    TIMSK3 |= (1 << OCIE1A);
  }else if (Axis == 1){
    TIMSK4 |= (1 << OCIE1A);
  }else if (Axis == 2){
    TIMSK5 |= (1 << OCIE1A);
  }
}

void TimerInitialisieren(){
  //https://www.robotshop.com/community/forum/t/arduino-101-timers-and-interrupts/13072
   noInterrupts();           // disable all interrupts
   if (CALCULATE_ACCELERATION_VIA_INTERRUPT){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    //Timer Länge brrechnen
    //OCR5A = 62500;            // compare match register 16MHz/256/1Hz
    //16 = 1 microsekunde
    OCR1A = ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES;       // compare match register 16MHz/256/1Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS10);    // 256 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt  
   }



  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3  = 0;

  //Timer Länge brrechnen
  //OCR5A = 62500;            // compare match register 16MHz/256/1Hz
  //16 = 1 microsekunde
  OCR3A = 65535;            // compare match register 16MHz/256/1Hz
  TCCR3B |= (1 << WGM12);   // CTC mode
  TCCR3B |= (1 << CS10);    // 256 prescaler 
  TIMSK3 |= (1 << OCIE1A);  // enable timer compare interrupt  




  TCCR4A = 0;
  TCCR4B = 0;
  TCNT4  = 0;

  //Timer Länge brrechnen
  //OCR5A = 62500;            // compare match register 16MHz/256/1Hz
  //16 = 1 microsekunde
  OCR4A = 65535;            // compare match register 16MHz/256/1Hz
  TCCR4B |= (1 << WGM12);   // CTC mode
  TCCR4B |= (1 << CS10);    // 256 prescaler 
  TIMSK4 |= (1 << OCIE1A);  // enable timer compare interrupt  




  TCCR5A = 0;
  TCCR5B = 0;
  TCNT5  = 0;

  OCR5A = 65535;            // compare match register 16MHz/256/1Hz
  TCCR5B |= (1 << WGM12);   // CTC mode
  TCCR5B |= (1 << CS10);    // 256 prescaler 
  TIMSK5 |= (1 << OCIE1A);  // enable timer compare interrupt  
  stopAxis(0);
  stopAxis(1);
  stopAxis(2);
  interrupts();             // enable all interrupts

}