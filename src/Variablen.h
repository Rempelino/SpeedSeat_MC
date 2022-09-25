struct dt_newCommand{
    unsigned long int X_Position;
    unsigned long int Y_Position;
    unsigned long int Z_Position;
} newCommand;
enum movementType{movementFromZero, movementWithChangeOfDirection, movementExtension};

bool requestHome = false;
bool newCommandPositionAvailable = false;
unsigned long int millisLastCycle = false;
bool stoppingToChangeDirection = false;
bool killCalled = false;
unsigned int ErrorNumber = 0;
volatile bool executed_X = false;
volatile bool executed_Y = false;
volatile bool executed_Z = false;



unsigned long int getSteps(unsigned long int PositionInMillimeter){
    return PositionInMillimeter * STEPS_PER_MM;
}

unsigned long int getMillimeter(unsigned long int PositionInSteps){
    return PositionInSteps / STEPS_PER_MM;
}

