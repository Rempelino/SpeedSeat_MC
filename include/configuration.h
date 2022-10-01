#ifndef CONFIG_H
#define CONFIG_H

#define EXECUTE_COMMUICATION true
#define NO_HARDWARE false
#define DEBUG_COMMUNICATION false
#define SIMULATION false
#define ANALYZE_INPUTS false
#define ANALYZE_AXIS false
#define CALCULATE_ACCELERATION_VIA_INTERRUPT false
#define ALLOW_COMMAND_WHEN_AXIS_IS_ACTIVE true
#define GET_VALUES_ON_BOOTUP false
#define STEPS_PER_MM 40UL
#define PIN_ENABLE 51
#define PIN_BEEPER 53
#define PROTOCOL_LENGTH 8
#define ALLOW_MOVEMENT_AFTER_BOOTUP true
#define PREVENT_BAD_HOMING true

//Default Values
#define X_AXIS_HOMING_OFFSET 20
#define Y_AXIS_HOMING_OFFSET 20
#define Z_AXIS_HOMING_OFFSET 20

#define X_AXIS_MAX_POSITION (210 - X_AXIS_HOMING_OFFSET)
#define Y_AXIS_MAX_POSITION (210 - Y_AXIS_HOMING_OFFSET)
#define Z_AXIS_MAX_POSITION (200 - Z_AXIS_HOMING_OFFSET)

#define X_AXIS_ACCELLERATION 600
#define Y_AXIS_ACCELLERATION 600
#define Z_AXIS_ACCELLERATION 600

#define X_AXIS_MAX_SPEED 350
#define Y_AXIS_MAX_SPEED 350
#define Z_AXIS_MAX_SPEED 350




#define PROCESSOR ARDUINO_AVR_MEGA2560
#if PROCESSOR == ARDUINO_AVR_MEGA2560
    #define PROCESSOR_CYCLES_PER_MICROSECOND 16UL
#else
    #error "Unbekannter Prozessor"
#endif
#define ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS 4UL
#define ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES (ACCELERATION_RECALCULATION_PERIOD_IN_MILLISECONDS * 1000UL * PROCESSOR_CYCLES_PER_MICROSECOND)

#include "pins.h"





#endif



