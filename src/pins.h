//Allgemeine Pins
#define PIN_ENABLE 51
#define PIN_BEEPER 53

//X-Axis
#define PIN_X_STEP 26
#define PIN_X_DIRECTION 27
#define PIN_X_ENABLE 28
#define PIN_X_TROUBLE 30
#define PIN_X_IN_POSITION 29
#define PIN_X_ENDSTOP 32

//Y-Axis
#define PIN_Y_STEP 14
#define PIN_Y_DIRECTION 15
#define PIN_Y_ENABLE 16
#define PIN_Y_TROUBLE 18
#define PIN_Y_IN_POSITION 17
#define PIN_Y_ENDSTOP 19

//Z-Axis
#define PIN_Z_STEP 2
#define PIN_Z_DIRECTION 3
#define PIN_Z_ENABLE 4
#define PIN_Z_TROUBLE 6
#define PIN_Z_IN_POSITION 5
#define PIN_Z_ENDSTOP 7






/*
struct Achse Y_Axis{    15,      //Direction
                        16,      //Enable
                        18,      //Trouble
                        17,      //InPosition
                        19,      //Endstop

                        210*STEPS_PER_MM,      //MaxPosition
                        600*STEPS_PER_MM,      //acceleration
                        600*STEPS_PER_MM/PROCESSOR_CYCLES_PER_MICROSECOND*ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES/1000000,//accelerationPerAccelerationRecalculation
                        400*STEPS_PER_MM,      //maxSpeed
                        (Y_Axis.maxSpeed * Y_Axis.maxSpeed) / (2*Y_Axis.acceleration),//DistanzAbbremsenVonMaxSpeed
                        .HomingOffset = 20,

                        0,      //aktiv
                        0,      //accelerating
                        0,      //deccelerating
                        0,      //homingAbgeschlossen
                        0,      //changeOfDirection
                        0,      //runningMinSpeed
                        0,      //currentSpeed

                        0,      //posStartDeccelerating
                        0,      //posStartDeccelerating_nextMove
                        0,      //istPosition
                        0,      //sollPosition

                        1,      //currentDirection
                        1,      //RundungsFehlerProAccellerationCalculation
                        0,      //RundungsfehlerSumiert
                        0,      //microsSinceLastAccelerationCalculation

                        &OCR4A, //TimerPeriod
                        &PORTJ, //Port
                        &DDRJ,  //OutpuRegister
                        PJ1,    //StepPinNumber

                        0,      //toggle
                        65535,   //stepPeriodInProcessorCycles
                        1       //AxisNomber
                        };


    */


