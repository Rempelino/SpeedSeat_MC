
#include "Arduino.h"
#include "configuration.h"
#include "Axis.h"
#include "communication.h"
#include "Beeping.h"

const unsigned long intervall = 20;
const unsigned long intervall1 = 100;
unsigned long timeStamp;
unsigned long timeStamp1;

communication com;
Axis X_Axis(PIN_X_STEP, PIN_X_DIRECTION, PIN_X_ENABLE, PIN_X_ENDSTOP, PIN_X_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);
Axis Y_Axis(PIN_Y_STEP, PIN_Y_DIRECTION, PIN_Y_ENABLE, PIN_Y_ENDSTOP, PIN_Y_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);
Axis Z_Axis(PIN_Z_STEP, PIN_Z_DIRECTION, PIN_Z_ENABLE, PIN_Z_ENDSTOP, PIN_Z_TROUBLE, _MC_FOCUS_ON_SMOOTHENESS);

Beeping beep(PIN_BEEPER, 400, 1000);
int AxisInBearbeitung;
void analyzeMotionCernel();
void setup()
{
  Serial.begin(38400);

  // Achse initialisieren
  Axis::ExecutePointer[0] = []()
  { X_Axis.execute(); };
  Axis::ExecutePointer[1] = []()
  { Y_Axis.execute(); };
  Axis::ExecutePointer[2] = []()
  { Z_Axis.execute(); };
  X_Axis.setStepsPerMillimeter(40);
  Y_Axis.setStepsPerMillimeter(40);
  Z_Axis.setStepsPerMillimeter(40);

  pinMode(56, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_ENABLE, INPUT_PULLUP);
  pinMode(PIN_BEEPER, OUTPUT);

  // TOBI die Zeiten einkommentieren sollte nich für Timeout sorgen
  digitalWrite(PIN_BEEPER, HIGH);
  // delay(500);
  digitalWrite(PIN_BEEPER, LOW);
  // delay(100);

  while (!digitalRead(PIN_ENABLE) & !NO_HARDWARE & !ALLOW_MOVEMENT_AFTER_BOOTUP)
  {
    beep.doubleBeep();
  }

  while (digitalRead(PIN_ENABLE) & !NO_HARDWARE)
  {
    delay(100);
  }
  timeStamp = millis();
  timeStamp1 = millis();
}

//--------------------------------------LOOP-------------------------------------------------
void loop()
{

  while (ANALYZE_MOTION_CERNEL)
  {
    analyzeMotionCernel();
  }

  if (EXECUTE_COMMUICATION)
  {
    com.execute();
    if (com.getRequestedValue() != IDLE)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      switch (com.getRequestedValue())
      {
      case POSITION:
        com.fillValueBuffer(
            X_Axis.getIstpositon() * STEPS_PER_MM * 0xFFFF / X_Axis.getMaxPosition(),
            Y_Axis.getIstpositon() * STEPS_PER_MM * 0xFFFF / Y_Axis.getMaxPosition(),
            Z_Axis.getIstpositon() * STEPS_PER_MM * 0xFFFF / Z_Axis.getMaxPosition());
        break;

      case MAX_POSITION:
        com.fillValueBuffer(X_Axis.getMaxPosition(), Y_Axis.getMaxPosition(), Z_Axis.getMaxPosition());
        break;

      case HOMING_OFFSET:
        com.fillValueBuffer(X_Axis.getHomingOffset(), Y_Axis.getHomingOffset(), Z_Axis.getHomingOffset());
        break;

      case ACCELLERATION:
        com.fillValueBuffer(X_Axis.getAcceleration(), Y_Axis.getAcceleration(), Z_Axis.getAcceleration());
        break;

      case MAX_SPEED:
        com.fillValueBuffer(X_Axis.getMaxSpeed(), Y_Axis.getMaxSpeed(), Z_Axis.getMaxSpeed());
        break;

      case HOMING_STATUS:
        com.fillValueBuffer(X_Axis.isHomed(), Y_Axis.isHomed(), Z_Axis.isHomed());
        break;

      case HOMING_SPEED:
        com.fillValueBuffer(X_Axis.getHomingSpeed(), Y_Axis.getHomingSpeed(), Z_Axis.getHomingSpeed());
        break;

      case HOMING_ACCELERATION:
        com.fillValueBuffer(X_Axis.getHomingAcceleration(), Y_Axis.getHomingAcceleration(), Z_Axis.getHomingAcceleration());
        break;

      case FPS:
        com.fillValueBuffer(com.fps, Axis::getWorkload() * 100, 0);
        break;

      case INIT_SUCCESSFUL:
        com.fillValueBuffer(0, 0, 0);
        break;

      default:
        break;
      }
    }
  }

  if (com.recived_value.is_available)
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    switch (com.recived_value.command)
    {
    case POSITION:
      X_Axis.moveAbsoluteInternal((unsigned long)(com.recived_value.as_int16[0]) * X_Axis.getMaxPosition() * STEPS_PER_MM / 0xFFFFul);
      Y_Axis.moveAbsoluteInternal((unsigned long)(com.recived_value.as_int16[1]) * Y_Axis.getMaxPosition() * STEPS_PER_MM / 0xFFFFul);
      Z_Axis.moveAbsoluteInternal((unsigned long)(com.recived_value.as_int16[2]) * Z_Axis.getMaxPosition() * STEPS_PER_MM / 0xFFFFul);
      break;

    case MAX_POSITION:
      X_Axis.setMaxPosition(com.recived_value.as_int16[0]);
      Y_Axis.setMaxPosition(com.recived_value.as_int16[1]);
      Z_Axis.setMaxPosition(com.recived_value.as_int16[2]);
      break;

    case HOMING_OFFSET:
      X_Axis.setHomingOffset(com.recived_value.as_int16[0]);
      Y_Axis.setHomingOffset(com.recived_value.as_int16[1]);
      Z_Axis.setHomingOffset(com.recived_value.as_int16[2]);
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

    case HOMING_SPEED:
      X_Axis.setHomingSpeed(com.recived_value.as_int16[0]);
      Y_Axis.setHomingSpeed(com.recived_value.as_int16[1]);
      Z_Axis.setHomingSpeed(com.recived_value.as_int16[2]);
      break;

    case HOMING_ACCELERATION:
      X_Axis.setHomingAcceleration(com.recived_value.as_int16[0]);
      Y_Axis.setHomingAcceleration(com.recived_value.as_int16[1]);
      Z_Axis.setHomingAcceleration(com.recived_value.as_int16[2]);
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
      break;

    default:
      break;
    }
    com.recived_value.is_available = false;
  }

  if (NO_HARDWARE)
  {
    return;
  }

  // END HERE IF NO HARDWARE ---------------------------

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

  if ((X_Axis.hasError() || Y_Axis.hasError() || Z_Axis.hasError()))
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

  if (digitalRead(PIN_ENABLE) && Axis::steppingIsEnabled())
  {
    while (X_Axis.isActive() || Y_Axis.isActive() || Z_Axis.isActive())
    {
      X_Axis.stop();
      Y_Axis.stop();
      Z_Axis.stop();
    }
    Axis::disableStepping();
    beep.doubleBeep();
  }

  if (!digitalRead(PIN_ENABLE) && !Axis::steppingIsEnabled())
  {
    beep.doubleBeep();
    Axis::enableStepping();
  }
}

void analyzeMotionCernel()
{
  unsigned long myMillis = millis();

  if (myMillis - timeStamp > intervall)
  {
    X_Axis.printIstPosition();
    X_Axis.printSollPosition();
    X_Axis.printMaxPosition();
    // X_Axis.printPositionDeccelerating();
    Serial.println();
    Serial.flush();
    timeStamp += intervall;
  }

  if (myMillis - timeStamp1 > intervall1)
  {
    int x = random(0, 3);
    switch (x)
    {
    case 0:
      X_Axis.moveAbsolute(random(0, X_Axis.getMaxPosition()));
      break;

    case 1:
      X_Axis.moveAbsolute(X_Axis.getMaxPosition());
      break;

    case 2:
      X_Axis.moveAbsolute(0);
      break;

    default:
      break;
    }

    timeStamp1 += intervall1;
  }

  if (Serial.available() != 0)
  {
    while (Serial.available() != 0)
    {
      Serial.read();
    }
    while (Serial.available() == 0)
    {
    }
    while (Serial.available() != 0)
    {
      Serial.read();
    }
  }
  return;
}
