#ifndef AXISTIMER_H
#define AXISTIMER_H
// 3 Timer initialisieren für 3 Motoren (Timer 3,4,5 für Achse 0,1,2)
#ifndef SIMULATION
#define SIMULATION false
#endif
#ifndef CALCULATE_ACCELERATION_VIA_INTERRUPT
#define CALCULATE_ACCELERATION_VIA_INTERRUPT false
#endif
#include "Axxis.h"
#include "Arduino.h"

void Axxis::stopAxis(){
  if(SIMULATION){
    Serial.print("Stopping Axis: ");Serial.println(AxisNomber);
  }

  if (AxisNomber == 0){
    TIMSK3 = TIMSK3 & ~(1 << OCIE1A);
    OCR3A = 65535;
  }else if (AxisNomber == 1){
    TIMSK4 = TIMSK4 & ~(1 << OCIE1A);
    OCR4A = 65535;
  }else if (AxisNomber == 2){
    TIMSK5 = TIMSK5 & ~(1 << OCIE1A);
    OCR5A = 65535;
  }
}

void Axxis::startAxis(){
  if(SIMULATION){
    Serial.print("Axis: ");Serial.print(AxisNomber);Serial.println(" wird gestartet.");
  }

  if (AxisNomber == 0){
    TIMSK3 |= (1 << OCIE1A);
  }else if (AxisNomber == 1){
    TIMSK4 |= (1 << OCIE1A);
  }else if (AxisNomber == 2){
    TIMSK5 |= (1 << OCIE1A);
  }
}

void Axxis::TimerInitialisieren(){
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


  if (AxisNomber == 0){
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
  }

if (AxisNomber == 1){
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
}


if (AxisNomber == 2){
  TCCR5A = 0;
  TCCR5B = 0;
  TCNT5  = 0;

  OCR5A = 65535;            // compare match register 16MHz/256/1Hz
  TCCR5B |= (1 << WGM12);   // CTC mode
  TCCR5B |= (1 << CS10);    // 256 prescaler 
  TIMSK5 |= (1 << OCIE1A);  // enable timer compare interrupt  
}
  stopAxis();
  interrupts();             // enable all interrupts

}

#endif