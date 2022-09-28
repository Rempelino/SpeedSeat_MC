#define ABC

#ifndef ABC
int ErrorNumber;
#include "Arduino.h"
#include "configuration.h"
#include "Axxis.h"
#include "communication.h"
#include "beep.h"
#include "serialPrint.h"

Axxis X_Axis(PIN_X_DIRECTION,
             PIN_X_ENABLE,
             PIN_X_TROUBLE,
             PIN_X_IN_POSITION,
             PIN_ENABLE,
             PIN_X_ENDSTOP, BIT_PIN_26,
             0,
             X_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             X_AXIS_ACCELLERATION,
             X_AXIS_MAX_SPEED,
             X_AXIS_HOMING_OFFSET,
             &OCR3A,
             &PORT_PIN_26,
             &REGISTER_PIN_26);
Axxis Y_Axis(PIN_Y_DIRECTION,
             PIN_Y_ENABLE,
             PIN_Y_TROUBLE,
             PIN_Y_IN_POSITION,
             PIN_ENABLE,
             PIN_Y_ENDSTOP, BIT_PIN_14,
             1,
             Y_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             Y_AXIS_ACCELLERATION,
             Y_AXIS_MAX_SPEED,
             Y_AXIS_HOMING_OFFSET,
             &OCR4A,
             &PORT_PIN_14,
             &REGISTER_PIN_14);
Axxis Z_Axis(PIN_Z_DIRECTION,
             PIN_Z_ENABLE,
             PIN_Z_TROUBLE,
             PIN_Z_IN_POSITION,
             PIN_ENABLE,
             PIN_Z_ENDSTOP, BIT_PIN_2,
             2,
             Z_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             Z_AXIS_ACCELLERATION,
             Z_AXIS_MAX_SPEED,
             Z_AXIS_HOMING_OFFSET,
             &OCR5A,
             &PORT_PIN_2,
             &REGISTER_PIN_2);


ISR(TIMER3_COMPA_vect){X_Axis.newStep();}
ISR(TIMER4_COMPA_vect){X_Axis.newStep();}
ISR(TIMER5_COMPA_vect){X_Axis.newStep();}


communication com;

void setup()
{
  Serial.begin(38400);
  com.initialize(STEPS_PER_MM, X_AXIS_MAX_POSITION, Y_AXIS_MAX_POSITION, Z_AXIS_MAX_POSITION);
  while (DEBUG_COMMUNICATION)
  {
    com.execute();
  }
  Serial.println(Z_Axis.getSomeValue());
  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_BEEPER, OUTPUT);

  digitalWrite(PIN_BEEPER, HIGH);
  delay(500);
  digitalWrite(PIN_BEEPER, LOW);
  delay(100);
  while (!digitalRead(PIN_ENABLE) & !NO_HARDWARE & !ALLOW_MOVEMENT_AFTER_BOOTUP)
  {
    doubleBeep();
  }
  
  while (ANALYZE_INPUTS)
  {
    printInputStatus();
    delay(1000);
  }

  while (ANALYZE_AXIS)
  {
    X_Axis.dumpAxisParameter();
    Y_Axis.dumpAxisParameter();
    Z_Axis.dumpAxisParameter();
    delay(1000);
  }
  
  while (digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    delay(100);
  }

  if (SIMULATION)
  {
    X_Axis.dumpAxisParameter();
    Y_Axis.dumpAxisParameter();
    Z_Axis.dumpAxisParameter();
    printInputStatus();
  }

  if (!NO_HARDWARE)
  {
    X_Axis.home();
    Y_Axis.home();
    Z_Axis.home();
  }
}

void loop()
{
}
#endif

#ifdef ABC
#include "Arduino.h"
#include "configuration.h"
#include "Variablen.h"
#include "Timer.h"
#include "AxisDefinition.h"
#include "kill.h"
#include "createMovement.h"
#include "Steps.h"
#include "ISRs.h"
#include "serialPrint.h"
#include "homing.h"
#include "communication.h"
#include "beep.h"

communication com;
void setup()
{
  Serial.begin(38400);
  com.initialize(STEPS_PER_MM, X_AXIS_MAX_POSITION, Y_AXIS_MAX_POSITION, Z_AXIS_MAX_POSITION);
  while (DEBUG_COMMUNICATION)
  {
    com.execute();
  }
  TimerInitialisieren();
  initializeAxis(X_Axis);
  initializeAxis(Y_Axis);
  initializeAxis(Z_Axis);
  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_BEEPER, OUTPUT);

  digitalWrite(PIN_BEEPER, HIGH);
  delay(500);
  digitalWrite(PIN_BEEPER, LOW);
  delay(100);
  while (!digitalRead(PIN_ENABLE) & !NO_HARDWARE & !ALLOW_MOVEMENT_AFTER_BOOTUP)
  {
    doubleBeep();
  }

  while (ANALYZE_INPUTS)
  {
    printInputStatus();
    delay(1000);
  }

  while (ANALYZE_AXIS)
  {
    dumpAxisParameter();
    delay(1000);
  }

  while (digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    delay(100);
  }
  digitalWrite(X_Axis.Pin.Enable, LOW);
  digitalWrite(Y_Axis.Pin.Enable, LOW);
  digitalWrite(Z_Axis.Pin.Enable, LOW);

  if (SIMULATION)
  {
    dumpAxisParameter();
    printInputStatus();
  }

  if (!NO_HARDWARE)
  {
    home();
  }
}
const bool HenryMoechteSichBewegen = false;
int AxisInBearbeitung = 1;
int Step;
//---------------------------------------------------------------------------------------
void loop()
{
  if (EXECUTE_COMMUICATION)
  {
    com.execute();
  }

  if (ALLOW_COMMAND_WHEN_AXIS_IS_ACTIVE || (!X_Axis.aktiv & !Y_Axis.aktiv & !Z_Axis.aktiv))
  {
    if (com.recived_value.is_available)
    {
      switch (com.recived_value.command)
      {
      case POSITION:
        move(0, com.recived_value.scaled_to_max_axis_pos_as_steps[0]);
        move(1, com.recived_value.scaled_to_max_axis_pos_as_steps[1]);
        move(2, com.recived_value.scaled_to_max_axis_pos_as_steps[2]);
        break;

      case HOMING_OFFSET:
        X_Axis.HomingOffset = com.recived_value.as_int16[0];
        Y_Axis.HomingOffset = com.recived_value.as_int16[1];
        Z_Axis.HomingOffset = com.recived_value.as_int16[2];
        requestHome = true;
        break;

      case MAX_POSITION:
        X_Axis.MaxPosition = com.recived_value.scaled_to_steps[0];
        Y_Axis.MaxPosition = com.recived_value.scaled_to_steps[1];
        Z_Axis.MaxPosition = com.recived_value.scaled_to_steps[2];
        com.setMaxPosition(com.recived_value.as_int16[0], com.recived_value.as_int16[1], com.recived_value.as_int16[2]);
        break;

      case ACCELLERATION:
        X_Axis.acceleration = com.recived_value.scaled_to_steps[0];
        Y_Axis.acceleration = com.recived_value.scaled_to_steps[1];
        Z_Axis.acceleration = com.recived_value.scaled_to_steps[2];
        break;

      default:
        break;
      }
      com.recived_value.is_available = false;
    }
  }
  // if (!X_Axis.aktiv){
  //  makeCustomMove();
  //}
  if (SIMULATION)
  {
    simulationPrint();
    if (Serial.available() != 0)
    {
      int t = Serial.read();
      Serial.println(t);
      if (t == 49)
      {
        AxisInBearbeitung = 0;
      } // 1
      if (t == 50)
      {
        AxisInBearbeitung = 1;
      } // 2
      if (t == 51)
      {
        AxisInBearbeitung = 2;
      } // 3
      if (t == 52)
      {
        move(1, Y_Axis.MaxPosition);
      } // 4
      if (t == 53)
      {
        move(1, 0);
      } // 5
      if (t == 54)
      {
        makeCustomMove();
      } // 6
      if (t == 43)
      {
        if (AxisInBearbeitung == 0)
        {
          move(AxisInBearbeitung, X_Axis.istPosition + getSteps(10));
        }
        if (AxisInBearbeitung == 1)
        {
          move(AxisInBearbeitung, Y_Axis.istPosition + getSteps(10));
        }
        if (AxisInBearbeitung == 2)
        {
          move(AxisInBearbeitung, Z_Axis.istPosition + getSteps(10));
        }
      }
      if (t == 45)
      {
        if (AxisInBearbeitung == 0)
        {
          move(AxisInBearbeitung, X_Axis.istPosition - getSteps(10));
        }
        if (AxisInBearbeitung == 1)
        {
          move(AxisInBearbeitung, Y_Axis.istPosition - getSteps(10));
        }
        if (AxisInBearbeitung == 2)
        {
          move(AxisInBearbeitung, Z_Axis.istPosition - getSteps(10));
        }
      }
    }
  }

  if (HenryMoechteSichBewegen)
  {
    if (!X_Axis.aktiv & !Y_Axis.aktiv & !Z_Axis.aktiv)
    {
      switch (Step)
      {
      case 0:
        move(0, X_Axis.MaxPosition - 1);
        Step++;
        break;

      case 1:
        move(1, Y_Axis.MaxPosition - 1);
        Step++;
        break;

      case 2:
        move(2, Z_Axis.MaxPosition - 1);
        Step++;
        break;

      case 3:
        move(0, 0);
        Step++;
        break;

      case 4:
        move(1, 0);
        Step++;
        break;

      case 5:
        move(2, 0);
        Step++;
        break;
      default:
        // HenryMoechteSichBewegen = false;
        Step = 0;
        break;
      }
    }
  }

  if (digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    stopAxis(0);
    stopAxis(1);
    stopAxis(2);
    X_Axis.aktiv = false;
    Y_Axis.aktiv = false;
    Z_Axis.aktiv = false;
    while (digitalRead(PIN_ENABLE))
    {
      delay(100);
    }
    home();
  }
  checkForKill();
  if (requestHome)
  {
    requestHome = false;
    home();
  }
}

#endif