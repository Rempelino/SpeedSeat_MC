#include "configuration.h"
#ifdef NO_HARDWARE
#warning NO_HARDWARE is defined. Do not push to chair.
#endif
#ifdef ANALYZE_MOTION_CERNEL
#warning ANALYZE_MOTION_CERNEL is defined. Axis will move independently to inputs!
#endif
#ifdef ALLOW_MOVEMENT_AFTER_BOOTUP
#warning ALLOW_MOVEMENT_AFTER_BOOTUP is defined. Disable in final builf or safety.
#endif
#ifdef PUSH_STATUS_WHEN_COM_IN_IDLE
#warning PUSH_STATUS_WHEN_COM_IN_IDLE is defined. Chair state will not be pushed to user interface.
#endif
#ifndef USE_EEPROM
#warning USE_EEPROM is not defined. Values will reset on every reboot.
#endif