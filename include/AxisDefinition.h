#ifndef AXISDEFINITION_H
#define AXISDEFINITION_H
#include "pins.h"
#include "Axxis.h"
#include "configuration.h"

Axxis X_Axis(PIN_X_DIRECTION,
             PIN_X_ENABLE,
             PIN_X_TROUBLE,
             PIN_X_IN_POSITION,
             PIN_X_ENDSTOP,
             PIN_ENABLE,
             BIT_PIN_26,
             0,
             X_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             X_AXIS_ACCELLERATION,
             X_AXIS_MAX_SPEED,
             X_AXIS_HOMING_OFFSET,
             &OCR3A,
             &PORT_PIN_26,
             &REGISTER_PIN_26);
ISR(TIMER3_COMPA_vect) { X_Axis.newStep(); }

Axxis Y_Axis(PIN_Y_DIRECTION,
             PIN_Y_ENABLE,
             PIN_Y_TROUBLE,
             PIN_Y_IN_POSITION,
             PIN_Y_ENDSTOP,
             PIN_ENABLE,
             BIT_PIN_14,
             1,
             Y_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             Y_AXIS_ACCELLERATION,
             Y_AXIS_MAX_SPEED,
             Y_AXIS_HOMING_OFFSET,
             &OCR4A,
             &PORT_PIN_14,
             &REGISTER_PIN_14);
ISR(TIMER4_COMPA_vect) { Y_Axis.newStep(); }

Axxis Z_Axis(PIN_Z_DIRECTION,
             PIN_Z_ENABLE,
             PIN_Z_TROUBLE,
             PIN_Z_IN_POSITION,
             PIN_Z_ENDSTOP,
             PIN_ENABLE,
             BIT_PIN_2,
             2,
             Z_AXIS_MAX_POSITION,
             STEPS_PER_MM,
             Z_AXIS_ACCELLERATION,
             Z_AXIS_MAX_SPEED,
             Z_AXIS_HOMING_OFFSET,
             &OCR5A,
             &PORT_PIN_2,
             &REGISTER_PIN_2);
ISR(TIMER5_COMPA_vect) { Z_Axis.newStep(); }

#endif