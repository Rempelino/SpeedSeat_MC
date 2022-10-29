#include "Arduino.h"
#include "configuration.h"
#include "Axis.h"
#include "communication.h"
#include "Beeping.h"

communication com;
Axis X_Axis(PIN_X_STEP, PIN_X_DIRECTION, PIN_X_ENABLE, PIN_X_ENDSTOP, PIN_X_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);
Axis Y_Axis(PIN_Y_STEP, PIN_Y_DIRECTION, PIN_Y_ENABLE, PIN_Y_ENDSTOP, PIN_Y_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);
Axis Z_Axis(PIN_Z_STEP, PIN_Z_DIRECTION, PIN_Z_ENABLE, PIN_Z_ENDSTOP, PIN_Z_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);

Beeping beep(PIN_BEEPER, 400, 1000);
int AxisInBearbeitung;

void setup()
{
  Serial.begin(38400);
  delay(300);
  pinMode(56, OUTPUT);
  digitalWrite(56, LOW);
  pinMode(60, OUTPUT);
  tone(60, 20, 500000);
  while (Serial.available() == 0)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  noTone(60);
  com.acknowledge(OKAY);
  delay(1000);
  com.sendInitFinishedCommand();
  while (DEBUG_COMMUNICATION)
  {
    com.execute();
    if (millis() > 5000)
    {
      com.sendValue(POSITION, 100, 200, 300);
      while (1)
      {
      }
    }
  }

  // threading c([](){com.execute(); },100);

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

  while (digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    delay(100);
  }
}
//--------------------------------------LOOP-------------------------------------------------
void loop()
{
  if (EXECUTE_COMMUICATION)
  {
    com.execute();
  }

  if (!Z_Axis.isHomed())
  {
    Z_Axis.home();
  }
  if (!X_Axis.isHomed() && Z_Axis.isHomed())
  {
    X_Axis.home();
  }
  if (!Y_Axis.isHomed() && Z_Axis.isHomed())
  {
    Y_Axis.home();
  }

  if (ALLOW_COMMAND_WHEN_AXIS_IS_ACTIVE || (!X_Axis.isActive() & !Y_Axis.isActive() & !Z_Axis.isActive()))
  {
    if (com.recived_value.is_available)
    {
      switch (com.recived_value.command)
      {
      case POSITION:
        X_Axis.moveAbsolute((unsigned long)(com.recived_value.as_int16[0]) * X_Axis.getMaxPosition() / 65535ul);
        Y_Axis.moveAbsolute((unsigned long)(com.recived_value.as_int16[1]) * X_Axis.getMaxPosition() / 65535ul);
        Z_Axis.moveAbsolute((unsigned long)(com.recived_value.as_int16[2]) * X_Axis.getMaxPosition() / 65535ul);
        break;

      case HOMING_OFFSET:
        X_Axis.setHomingOffset(com.recived_value.as_int16[0]);
        Y_Axis.setHomingOffset(com.recived_value.as_int16[1]);
        Z_Axis.setHomingOffset(com.recived_value.as_int16[2]);
        break;

      case MAX_POSITION:
        X_Axis.setMaxPosition(com.recived_value.as_int16[0]);
        Y_Axis.setMaxPosition(com.recived_value.as_int16[1]);
        Z_Axis.setMaxPosition(com.recived_value.as_int16[2]);
        break;

      case ACCELLERATION:
        X_Axis.setAcceleration(com.recived_value.as_int16[0]);
        Y_Axis.setAcceleration(com.recived_value.as_int16[1]);
        Z_Axis.setAcceleration(com.recived_value.as_int16[2]);
        break;

      case MAX_SPEED:
        X_Axis.setSpeed(com.recived_value.as_int16[0]);
        Y_Axis.setSpeed(com.recived_value.as_int16[1]);
        Z_Axis.setSpeed(com.recived_value.as_int16[2]);
        break;

      case NEW_HOMING:
        if (com.recived_value.as_bool[0])
        {
          X_Axis.home();
        }
        if (com.recived_value.as_bool[1])
        {
          Y_Axis.home();
        }
        if (com.recived_value.as_bool[2])
        {
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

  if (digitalRead(PIN_ENABLE) && !NO_HARDWARE)
  {
    while (X_Axis.isActive() || Y_Axis.isActive() || Z_Axis.isActive())
    {
      X_Axis.stop();
      Y_Axis.stop();
      Z_Axis.stop();
    }
    Axis::disableStepping();
  }

  if (!digitalRead(PIN_ENABLE) && !NO_HARDWARE)
  {
    beep.doubleBeep();
    Axis::enableStepping();
  }
}
