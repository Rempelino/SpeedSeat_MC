#include "Arduino.h"
#include "configuration.h"
#include "Axis.h"
#include "communication.h"
#include "Beeping.h"
#include "AxisDefinition.h"

communication com;
Beeping beep(PIN_BEEPER, 400, 1000);
int AxisInBearbeitung;

void setup()
{
  Serial.begin(38400);

  while(true){
    Serial.println(X_Axis.getSomeValue());
    Serial.println(Y_Axis.getSomeValue());
    Serial.println(Z_Axis.getSomeValue());
    delay(1000);
  }
  com.initialize(STEPS_PER_MM, X_AXIS_MAX_POSITION, Y_AXIS_MAX_POSITION, Z_AXIS_MAX_POSITION);
  while (DEBUG_COMMUNICATION)
  {
    com.execute();
  }

  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_BEEPER, OUTPUT);

  digitalWrite(PIN_BEEPER, HIGH);
  delay(500);
  digitalWrite(PIN_BEEPER, LOW);
  delay(100);

  while (!digitalRead(PIN_ENABLE) & !NO_HARDWARE & !ALLOW_MOVEMENT_AFTER_BOOTUP)
  {
    beep.doubleBeep();
  }

  while (ANALYZE_INPUTS)
  {
    X_Axis.printInputStatus();
    Y_Axis.printInputStatus();
    Z_Axis.printInputStatus();
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
  }
}
//--------------------------------------LOOP-------------------------------------------------
void loop()
{
  if (EXECUTE_COMMUICATION)
  {
    com.execute();
  }

  if (!Z_Axis.isHomed)
  {
    Z_Axis.home();
  }
  if (!X_Axis.isHomed)
  {
    X_Axis.home();
  }
  if (!Y_Axis.isHomed)
  {
    Y_Axis.home();
  }

  if (!X_Axis.isFullyInitialized())
  {
    com.get_value(X_Axis.missingValue());
  }
  if (!Y_Axis.isFullyInitialized())
  {
    com.get_value(Y_Axis.missingValue());
  }
  if (!Z_Axis.isFullyInitialized())
  {
    com.get_value(Z_Axis.missingValue());
  }

  if (ALLOW_COMMAND_WHEN_AXIS_IS_ACTIVE || (!X_Axis.aktiv & !Y_Axis.aktiv & !Z_Axis.aktiv))
  {
    if (com.recived_value.is_available)
    {
      switch (com.recived_value.command)
      {
      case POSITION:
        X_Axis.move(com.recived_value.scaled_to_max_axis_pos_as_steps[0]);
        Y_Axis.move(com.recived_value.scaled_to_max_axis_pos_as_steps[1]);
        Z_Axis.move(com.recived_value.scaled_to_max_axis_pos_as_steps[2]);
        break;

      case HOMING_OFFSET:
        X_Axis.setHomingOffset(com.recived_value.as_int16[0]);
        Y_Axis.setHomingOffset(com.recived_value.as_int16[1]);
        Z_Axis.setHomingOffset(com.recived_value.as_int16[2]);
        break;

      case MAX_POSITION:
        X_Axis.setMaxPosition(com.recived_value.scaled_to_steps[0]);
        Y_Axis.setMaxPosition(com.recived_value.scaled_to_steps[1]);
        Z_Axis.setMaxPosition(com.recived_value.scaled_to_steps[2]);
        com.setMaxPosition(com.recived_value.as_int16[0], com.recived_value.as_int16[1], com.recived_value.as_int16[2]);
        break;

      case ACCELLERATION:
        X_Axis.setAcceleration(com.recived_value.as_int16[0]);
        Y_Axis.setAcceleration(com.recived_value.as_int16[1]);
        Z_Axis.setAcceleration(com.recived_value.as_int16[2]);
        break;

      case MAX_SPEED:
        X_Axis.setMaxSpeed(com.recived_value.as_int16[0]);
        Y_Axis.setMaxSpeed(com.recived_value.as_int16[1]);
        Z_Axis.setMaxSpeed(com.recived_value.as_int16[2]);
        break;

      case NEW_HOMING:
        if(com.recived_value.as_bool[0]){
          X_Axis.home();
        }
        if(com.recived_value.as_bool[1]){
          Y_Axis.home();
        }
        if(com.recived_value.as_bool[2]){
          Z_Axis.home();
        }

      default:
        break;
      }
      com.recived_value.is_available = false;
    }
  }

  if ((X_Axis.hasError() || Y_Axis.hasError() || Z_Axis.hasError()) && !NO_HARDWARE)
  {
    X_Axis.unlock();
    Y_Axis.unlock();
    Z_Axis.unlock();
    while (!digitalRead(PIN_ENABLE))
    {
      if (X_Axis.hasError())
      {
        beep.beep(X_Axis.getErrorID());
      }
      if (Y_Axis.hasError())
      {
        beep.beep(Y_Axis.getErrorID() + 10);
      }
      if (Z_Axis.hasError())
      {
        beep.beep(Z_Axis.getErrorID() + 20);
      }
    }
    beep.kill();
    while (digitalRead(PIN_ENABLE))
    {
    }
    X_Axis.lock();
    Y_Axis.lock();
    Z_Axis.lock();
  }

  while (digitalRead(PIN_ENABLE) && !NO_HARDWARE)
  {
    X_Axis.stopAxis();
    Y_Axis.stopAxis();
    Z_Axis.stopAxis();
  }
  if (SIMULATION)
  {
    if (Serial.available() != 0)
    {
      X_Axis.printStatus();

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
        if (AxisInBearbeitung == 0)
        {
          X_Axis.move(X_Axis.MaxPosition);
        }
        if (AxisInBearbeitung == 1)
        {
          Y_Axis.move(Y_Axis.MaxPosition);
        }
        if (AxisInBearbeitung == 2)
        {
          Z_Axis.move(Z_Axis.MaxPosition);
        }
      } // 4
      if (t == 53)
      {
        if (AxisInBearbeitung == 0)
        {
          X_Axis.move(0);
        }
        if (AxisInBearbeitung == 1)
        {
          Y_Axis.move(0);
        }
        if (AxisInBearbeitung == 2)
        {
          Z_Axis.move(0);
        }
      } // 5
      if (t == 54)
      {
        // makeCustomMove();
      } // 6
      if (t == 43)
      {
        if (AxisInBearbeitung == 0)
        {
          X_Axis.move(X_Axis.istPosition + 10 * STEPS_PER_MM);
        }
        if (AxisInBearbeitung == 1)
        {
          Y_Axis.move(Y_Axis.istPosition + 10 * STEPS_PER_MM);
        }
        if (AxisInBearbeitung == 2)
        {
          Z_Axis.move(Z_Axis.istPosition + 10 * STEPS_PER_MM);
        }
      }
      if (t == 45)
      {
        if (AxisInBearbeitung == 0)
        {
          X_Axis.move(X_Axis.istPosition - 10 * STEPS_PER_MM);
        }
        if (AxisInBearbeitung == 1)
        {
          Y_Axis.move(Y_Axis.istPosition - 10 * STEPS_PER_MM);
        }
        if (AxisInBearbeitung == 2)
        {
          Z_Axis.move(Z_Axis.istPosition - 10 * STEPS_PER_MM);
        }
      }
    }
  }
}
