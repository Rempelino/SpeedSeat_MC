#include "configuration.h"
// Allgemeine Pins
#define PIN_ENABLE 51
#define PIN_BEEPER 53

#ifdef NO_HARDWARE
// X-Axis
#define PIN_X_STEP 60
#define PIN_X_DIRECTION 61
#define PIN_X_ENABLE 56
#define PIN_X_TROUBLE 30
#define PIN_X_IN_POSITION 29
#define PIN_X_ENDSTOP 32

// Y-Axis
#define PIN_Y_STEP 36
#define PIN_Y_DIRECTION 34
#define PIN_Y_ENABLE 30
#define PIN_Y_TROUBLE 18
#define PIN_Y_IN_POSITION 17
#define PIN_Y_ENDSTOP 19
#else

// X-Axis
#define PIN_X_STEP 26
#define PIN_X_DIRECTION 27
#define PIN_X_ENABLE 28
#define PIN_X_TROUBLE 30
#define PIN_X_IN_POSITION 29
#define PIN_X_ENDSTOP 32

// Y-Axis
#define PIN_Y_STEP 14
#define PIN_Y_DIRECTION 15
#define PIN_Y_ENABLE 16
#define PIN_Y_TROUBLE 18
#define PIN_Y_IN_POSITION 17
#define PIN_Y_ENDSTOP 19
#endif





// Z-Axis
#define PIN_Z_STEP 2
#define PIN_Z_DIRECTION 3
#define PIN_Z_ENABLE 4
#define PIN_Z_TROUBLE 6
#define PIN_Z_IN_POSITION 5
#define PIN_Z_ENDSTOP 7

