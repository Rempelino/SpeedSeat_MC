//Zuweisung der Outputregister
#if PROCESSOR == ARDUINO_AVR_MEGA2560

    #define PORT_PIN_2 PORTE
    #define REGISTER_PIN_2 DDRE
    #define BIT_PIN_2 PE4

    #define PORT_PIN_14 PORTJ
    #define REGISTER_PIN_14 DDRJ
    #define BIT_PIN_14 PJ1

    #define PORT_PIN_26 PORTA
    #define REGISTER_PIN_26 DDRA
    #define BIT_PIN_26 PA4
#else
    #error "Unbekannter Prozessor"
#endif



#define X_AXIS_INTERRUPT OCR3A
#if PIN_X_STEP == 26
    #define X_AXIS_PORT PORT_PIN_26
    #define X_AXIS_REGISTER REGISTER_PIN_26
    #define X_AXIS_BIT BIT_PIN_26
#else
    #error #error "Port, Register und Pinzuweisung konnte nicht gefunden werden. Weitere Infos unter https://docs.arduino.cc/static/ed4d33b86bac66d54959fbb186795a11/54c3a/PinMap2560big_Rev2.png "
#endif

#define Y_AXIS_INTERRUPT OCR4A
#if PIN_Y_STEP == 14
    #define Y_AXIS_PORT PORT_PIN_14
    #define Y_AXIS_REGISTER REGISTER_PIN_14
    #define Y_AXIS_BIT BIT_PIN_14
#else
    #error #error "Port, Register und Pinzuweisung konnte nicht gefunden werden. Weitere Infos unter https://docs.arduino.cc/static/ed4d33b86bac66d54959fbb186795a11/54c3a/PinMap2560big_Rev2.png "
#endif

#define Z_AXIS_INTERRUPT OCR5A
#if PIN_Z_STEP == 2
    #define Z_AXIS_PORT PORT_PIN_2
    #define Z_AXIS_REGISTER REGISTER_PIN_2
    #define Z_AXIS_BIT BIT_PIN_2
#else
    #error #error "Port, Register und Pinzuweisung konnte nicht gefunden werden. Weitere Infos unter https://docs.arduino.cc/static/ed4d33b86bac66d54959fbb186795a11/54c3a/PinMap2560big_Rev2.png "
#endif



#ifndef PINCONFIG
#define PINCONFIG
struct PinConfig{
    const int Direction;
    const int Enable;
    const int Trouble;
    const int InPosition;
    const int Endstop;
};
#endif

struct Achse{
    const struct PinConfig Pin;
    unsigned long int MaxPosition;
    unsigned long int acceleration;
    const unsigned long int accelerationPerAccelerationRecalculation;
    const unsigned long int maxSpeed;
    const unsigned long int DistanzAbbremsenVonMaxSpeed;
    unsigned int HomingOffset;

    volatile bool aktiv;
    bool accelerating;
    bool deccelerating;
    bool homingAbgeschlossen;
    bool changeOfDirection;
    volatile bool runningMinSpeed;
    unsigned long int currentSpeed;

    unsigned long int posStartDeccelerating;
    unsigned long int sollPositionNachRichtungswechsel;
    volatile unsigned long int istPosition;
    unsigned long int sollPosition;

    bool currentDirection;
    const unsigned int RundungsFehlerProAccellerationCalculation;
    unsigned int RundungsfehlerSumiert;
    unsigned long int CyclesSinceLastAccelerationCalculation;

    volatile uint16_t *TimerPeriod;
    volatile uint8_t *Port;
    volatile uint8_t *OutputRegister;
    const unsigned int StepPinNumber;

    bool toggle;
    unsigned int stepPeriodInProcessorCycles;
    const unsigned short int AxisNomber;
    
};

struct Achse X_Axis{    Pin : {
                            Direction : PIN_X_DIRECTION,
                            Enable : PIN_X_ENABLE,
                            Trouble : PIN_X_TROUBLE,
                            InPosition: PIN_X_IN_POSITION,
                            Endstop: PIN_X_ENDSTOP},

                        MaxPosition: X_AXIS_MAX_POSITION * STEPS_PER_MM,
                        acceleration: X_AXIS_ACCELLERATION * STEPS_PER_MM,
                        accelerationPerAccelerationRecalculation: X_Axis.acceleration / PROCESSOR_CYCLES_PER_MICROSECOND * ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES / 1000000,
                        maxSpeed: X_AXIS_MAX_SPEED * STEPS_PER_MM,      //maxSpeed
                        DistanzAbbremsenVonMaxSpeed: (X_Axis.maxSpeed * X_Axis.maxSpeed) / (2*X_Axis.acceleration),
                        HomingOffset: X_AXIS_HOMING_OFFSET,

                        aktiv: false,
                        accelerating: false,
                        deccelerating: false,
                        homingAbgeschlossen: false,
                        changeOfDirection: false,
                        runningMinSpeed: false,
                        currentSpeed: false,

                        posStartDeccelerating: 0,
                        sollPositionNachRichtungswechsel: 0,
                        istPosition: 0,
                        sollPosition: 0,

                        currentDirection: true,
                        RundungsFehlerProAccellerationCalculation: 1,
                        RundungsfehlerSumiert: 0,
                        CyclesSinceLastAccelerationCalculation: 0,

                        TimerPeriod: &X_AXIS_INTERRUPT,
                        Port: &X_AXIS_PORT,
                        OutputRegister: &X_AXIS_REGISTER,
                        StepPinNumber: X_AXIS_BIT,

                        toggle: false,
                        stepPeriodInProcessorCycles: 65535,
                        AxisNomber: 0
                        };

struct Achse Y_Axis{    Pin : {
                            Direction : PIN_Y_DIRECTION,
                            Enable : PIN_Y_ENABLE,
                            Trouble : PIN_Y_TROUBLE,
                            InPosition: PIN_Y_IN_POSITION,
                            Endstop: PIN_Y_ENDSTOP},

                        MaxPosition: Y_AXIS_MAX_POSITION * STEPS_PER_MM,
                        acceleration: Y_AXIS_ACCELLERATION * STEPS_PER_MM,
                        accelerationPerAccelerationRecalculation: Y_Axis.acceleration / PROCESSOR_CYCLES_PER_MICROSECOND * ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES / 1000000,
                        maxSpeed: Y_AXIS_MAX_SPEED * STEPS_PER_MM,      //maxSpeed
                        DistanzAbbremsenVonMaxSpeed: (Y_Axis.maxSpeed * Y_Axis.maxSpeed) / (2*Y_Axis.acceleration),
                        HomingOffset: Y_AXIS_HOMING_OFFSET,

                        aktiv: false,
                        accelerating: false,
                        deccelerating: false,
                        homingAbgeschlossen: false,
                        changeOfDirection: false,
                        runningMinSpeed: false,
                        currentSpeed: false,

                        posStartDeccelerating: 0,
                        sollPositionNachRichtungswechsel: 0,
                        istPosition: 0,
                        sollPosition: 0,

                        currentDirection: true,
                        RundungsFehlerProAccellerationCalculation: 1,
                        RundungsfehlerSumiert: 0,
                        CyclesSinceLastAccelerationCalculation: 0,

                        TimerPeriod: &Y_AXIS_INTERRUPT,
                        Port: &Y_AXIS_PORT,
                        OutputRegister: &Y_AXIS_REGISTER,
                        StepPinNumber: Y_AXIS_BIT,

                        toggle: false,
                        stepPeriodInProcessorCycles: 65535,
                        AxisNomber: 1
                        };


struct Achse Z_Axis{    Pin : {
                            Direction : PIN_Z_DIRECTION,
                            Enable : PIN_Z_ENABLE,
                            Trouble : PIN_Z_TROUBLE,
                            InPosition: PIN_Z_IN_POSITION,
                            Endstop: PIN_Z_ENDSTOP},

                        MaxPosition: Z_AXIS_MAX_POSITION * STEPS_PER_MM,
                        acceleration: Z_AXIS_ACCELLERATION * STEPS_PER_MM,
                        accelerationPerAccelerationRecalculation: Z_Axis.acceleration / PROCESSOR_CYCLES_PER_MICROSECOND * ACCEL_RECALC_PERIOD_IN_PROCESSOR_CYLCES / 1000000,
                        maxSpeed: Z_AXIS_MAX_SPEED * STEPS_PER_MM,      //maxSpeed
                        DistanzAbbremsenVonMaxSpeed: (Z_Axis.maxSpeed * Z_Axis.maxSpeed) / (2*Z_Axis.acceleration),
                        HomingOffset: Z_AXIS_HOMING_OFFSET,

                        aktiv: false,
                        accelerating: false,
                        deccelerating: false,
                        homingAbgeschlossen: false,
                        changeOfDirection: false,
                        runningMinSpeed: false,
                        currentSpeed: false,

                        posStartDeccelerating: 0,
                        sollPositionNachRichtungswechsel: 0,
                        istPosition: 0,
                        sollPosition: 0,

                        currentDirection: true,
                        RundungsFehlerProAccellerationCalculation: 1,
                        RundungsfehlerSumiert: 0,
                        CyclesSinceLastAccelerationCalculation: 0,

                        TimerPeriod: &Z_AXIS_INTERRUPT,
                        Port: &Z_AXIS_PORT,
                        OutputRegister: &Z_AXIS_REGISTER,
                        StepPinNumber: Z_AXIS_BIT,

                        toggle: false,
                        stepPeriodInProcessorCycles: 65535,
                        AxisNomber: 2
                        };





struct Achse *Axis;

void initializeAxis(struct Achse Axis){
    pinMode(Axis.Pin.Direction,OUTPUT);
    pinMode(Axis.Pin.Enable,OUTPUT);
    pinMode(Axis.Pin.InPosition,INPUT_PULLUP);
    pinMode(Axis.Pin.Trouble,INPUT_PULLUP);
    pinMode(Axis.Pin.Endstop,INPUT_PULLUP);

    digitalWrite(Axis.Pin.Direction,LOW);
    digitalWrite(Axis.Pin.Enable,LOW);
    uint8_t AktuellerWertPort = *Axis.OutputRegister;
    *Axis.OutputRegister = AktuellerWertPort |(1<<Axis.StepPinNumber);
}

struct Achse *getAxis(unsigned short int AxisNomber){
    if (AxisNomber == 0){
        return &X_Axis;
    }else if(AxisNomber == 1){
        return &Y_Axis;
    }else{
        return &Z_Axis;
    }
}