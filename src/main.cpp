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
  while (!digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    doubleBeep();
  }

  if (ANALYZE_INPUTS)
  {
    while (true)
    {
      printInputStatus();
      delay(1000);
    }
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
        break;

      case MAX_POSITION:
        X_Axis.MaxPosition = com.recived_value.scaled_to_steps[0];
        Y_Axis.MaxPosition = com.recived_value.scaled_to_steps[1];
        Z_Axis.MaxPosition = com.recived_value.scaled_to_steps[2];
        com.initialize(STEPS_PER_MM, com.recived_value.as_int16[0], com.recived_value.as_int16[1], com.recived_value.as_int16[2]);
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
    home();
    requestHome = false;
  }
}