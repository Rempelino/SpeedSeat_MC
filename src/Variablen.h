struct dt_newCommand{
    unsigned long X_Position;
    unsigned long Y_Position;
    unsigned long Z_Position;
} newCommand;
enum movementType{movementFromZero, movementWithChangeOfDirection, movementExtension};

bool requestHome = false;
bool newCommandPositionAvailable = false;
unsigned long millisLastCycle = false;
bool stoppingToChangeDirection = false;
bool killCalled = false;
unsigned int ErrorNumber = 0;



unsigned long getSteps(unsigned long PositionInMillimeter){
    return PositionInMillimeter * STEPS_PER_MM;
}

unsigned long getMillimeter(unsigned long PositionInSteps){
    return PositionInSteps / STEPS_PER_MM;
}

