#include "Arduino.h"
#include "configuration.h"
#include "Axxis.h"
#include "communication.h"
#include "Beeping.h"
#include "AxisDefinition.h"

communication com;
Beeping beep(PIN_BEEPER, 400, 1000);

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

  if (!NO_HARDWARE)
  {
    X_Axis.home();
    Y_Axis.home();
    Z_Axis.home();
  }
}
//--------------------------------------LOOP-------------------------------------------------
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
        X_Axis.MaxPosition = com.recived_value.scaled_to_steps[0];
        Y_Axis.MaxPosition = com.recived_value.scaled_to_steps[1];
        Z_Axis.MaxPosition = com.recived_value.scaled_to_steps[2];
        com.setMaxPosition(com.recived_value.as_int16[0], com.recived_value.as_int16[1], com.recived_value.as_int16[2]);
        break;

      case ACCELLERATION:
        X_Axis.setAcceleration(com.recived_value.scaled_to_steps[0]);
        Y_Axis.setAcceleration(com.recived_value.scaled_to_steps[1]);
        Z_Axis.setAcceleration(com.recived_value.scaled_to_steps[2]);
        break;

      default:
        break;
      }
      com.recived_value.is_available = false;
    }
  }


  if (X_Axis.hasError() || Y_Axis.hasError() || Z_Axis.hasError())
  {
    X_Axis.unlock();
    Y_Axis.unlock();
    Z_Axis.unlock();
    while (!digitalRead(PIN_ENABLE))
    {
      beep.beep(X_Axis.getErrorID());
      beep.beep(Y_Axis.getErrorID() + 10);
      beep.beep(Z_Axis.getErrorID() + 20);
    }
    while (digitalRead(PIN_ENABLE))
    {
    }
    X_Axis.lock();
    Y_Axis.lock();
    Z_Axis.lock();
  }
}