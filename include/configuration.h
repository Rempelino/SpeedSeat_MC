#ifndef CONFIG_H
#define CONFIG_H

#define STEPS_PER_MM 40UL

//#define NO_HARDWARE
//#define ANALYZE_MOTION_CERNEL
//#define ALLOW_MOVEMENT_AFTER_BOOTUP
#define USE_EEPROM
//#define AUTO_SAVE
#define AUTO_RETURN_TO_ZERO
#include "pins.h"
#endif

#define MOVEMENT_NOT_ALLOWED (Axis::digitalReadAverage(PIN_ENABLE))
#define MOVEMENT_ALLOWED (!Axis::digitalReadAverage(PIN_ENABLE))




