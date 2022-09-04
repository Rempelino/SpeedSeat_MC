#include "commandList.h"

#define PROTOCOL_LENGTH 8
#define TIMEOUT 1000
#define TIMEOUT_ACTIVE true

CMD request = IDLE;
#if GET_VALUES_ON_BOOTUP
    CMD unknownValue = HOMING_OFFSET;
#else
    CMD unknownValue = IDLE;
#endif

byte buffer[PROTOCOL_LENGTH];
bool waitingForOKAY = false;
unsigned long millisAtSendingAnswer;

void acknowledge(ANSWER);
unsigned long int TwoBytesToSteps(byte, byte, unsigned long int);
bool commandFound(byte);
bool verifyData();
void setPosition();
void readNewCommand();
void unsignedLongToTwoBytes(unsigned long int, unsigned long int, byte*, byte*);
void sendBuffer();
void sendAnswer();
void sendValueRequest();
void setNextValue();

void executeCommunication()
{   
    if ((unknownValue != IDLE) & (Serial.available() == 0) & !waitingForOKAY){
        sendValueRequest();
    }

    if (waitingForOKAY & ((Serial.available() == 1) || (Serial.available() == PROTOCOL_LENGTH + 1)))
    {
        waitingForOKAY = false;
        if (Serial.read() == 255)
        {
            request = IDLE;
        }
        acknowledge(NO_ANSWER);
    }

    if (!waitingForOKAY & (request != IDLE))
    {   
        sendAnswer();
    }

    if (!waitingForOKAY & (Serial.available() == PROTOCOL_LENGTH) & (request == IDLE))
    {
        readNewCommand();
    }

    if (waitingForOKAY & TIMEOUT_ACTIVE){
        if (millis() - millisAtSendingAnswer > TIMEOUT){
            sendBuffer();
        }
    }

    if(!waitingForOKAY & (Serial.available() > PROTOCOL_LENGTH)){
        acknowledge(NOT_OKAY);
    }
}


void readNewCommand()
{
    int i;
    for (i = 0; i < PROTOCOL_LENGTH; i++)
    {
        buffer[i] = Serial.read();
    }

    if (!verifyData())
    {
        acknowledge(NOT_OKAY);
        return;
    }

    bool successfulExecuted = true;
    CMD command = (CMD)(buffer[0] / 2);
    bool reading = (bool)(buffer[0] % 2);
    switch (command)
    {
    case POSITION:
        if (reading)
        {
            request = command;
        }
        else
        {
            newCommand.X_Position = TwoBytesToSteps(buffer[1], buffer[2], X_Axis.MaxPosition);
            newCommand.Y_Position = TwoBytesToSteps(buffer[3], buffer[4], Y_Axis.MaxPosition);
            newCommand.Z_Position = TwoBytesToSteps(buffer[5], buffer[6], Z_Axis.MaxPosition);
            newCommandPositionAvailable = true;
        }
        break;
    
    case HOMING_OFFSET:
        if (reading)
        {
            request = command;
        }else{
            X_Axis.HomingOffset = TwoBytesToSteps(buffer[1], buffer[2], 0);
            Y_Axis.HomingOffset = TwoBytesToSteps(buffer[3], buffer[4], 0);
            Z_Axis.HomingOffset = TwoBytesToSteps(buffer[5], buffer[6], 0);
            requestHome = true;
        }
        break;

    case MAX_POSITION:
        if (reading)
        {
            request = command;
        }else{
            X_Axis.MaxPosition = TwoBytesToSteps(buffer[1], buffer[2], 0) * STEPS_PER_MM;
            Y_Axis.MaxPosition = TwoBytesToSteps(buffer[3], buffer[4], 0) * STEPS_PER_MM;
            Z_Axis.MaxPosition = TwoBytesToSteps(buffer[5], buffer[6], 0) * STEPS_PER_MM;
            requestHome = true;
        }
        break;

    default:
        successfulExecuted = false;
        break;
    }

    if (successfulExecuted){
        acknowledge(OKAY);
        if (reading){
            sendAnswer();
        }
    }else{
        acknowledge(NOT_OKAY);
    }
}

void sendAnswer(){
    memset(buffer, 0, PROTOCOL_LENGTH);
    buffer[0] = int(request) * 2;
    switch (request)
    {
    case POSITION:
        unsignedLongToTwoBytes(X_Axis.istPosition, X_Axis.MaxPosition, &buffer[1], &buffer[2]);
        unsignedLongToTwoBytes(Y_Axis.istPosition, Y_Axis.MaxPosition, &buffer[3], &buffer[4]);
        unsignedLongToTwoBytes(Z_Axis.istPosition, Z_Axis.MaxPosition, &buffer[5], &buffer[6]);
        break;

    case HOMING_OFFSET:
        unsignedLongToTwoBytes(X_Axis.HomingOffset, 0, &buffer[1], &buffer[2]);
        unsignedLongToTwoBytes(Y_Axis.HomingOffset, 0, &buffer[3], &buffer[4]);
        unsignedLongToTwoBytes(Z_Axis.HomingOffset, 0, &buffer[5], &buffer[6]);
        break;

    case MAX_POSITION:
        unsignedLongToTwoBytes(X_Axis.MaxPosition / STEPS_PER_MM, 0, &buffer[1], &buffer[2]);
        unsignedLongToTwoBytes(Y_Axis.MaxPosition / STEPS_PER_MM, 0, &buffer[3], &buffer[4]);
        unsignedLongToTwoBytes(Z_Axis.MaxPosition / STEPS_PER_MM, 0, &buffer[5], &buffer[6]);
        break;

    case ACCELLERATION:
        unsignedLongToTwoBytes(X_Axis.acceleration, STEPS_PER_MM, &buffer[1], &buffer[2]);
        unsignedLongToTwoBytes(Y_Axis.acceleration, STEPS_PER_MM, &buffer[3], &buffer[4]);
        unsignedLongToTwoBytes(Z_Axis.acceleration, STEPS_PER_MM, &buffer[5], &buffer[6]);
        break;

    case MAX_SPEED:
        unsignedLongToTwoBytes(X_Axis.maxSpeed, STEPS_PER_MM, &buffer[1], &buffer[2]);
        unsignedLongToTwoBytes(Y_Axis.maxSpeed, STEPS_PER_MM, &buffer[3], &buffer[4]);
        unsignedLongToTwoBytes(Z_Axis.maxSpeed, STEPS_PER_MM, &buffer[5], &buffer[6]);
        break;

    case HOMING_STATUS:
        buffer[1] = requestHome;
        break;

    case STEPS_PER_MILLIMETER:
        unsignedLongToTwoBytes(STEPS_PER_MM, 0, &buffer[1], &buffer[2]);
        break;

    default:
        request = IDLE;
        break;
    }
    if (request != IDLE){
        sendBuffer();
    }
}


void acknowledge(ANSWER answer)
{
    while (Serial.available() > 0)
    {
        Serial.read();
    }
    switch (answer)
    {
    case OKAY:
        Serial.write(255);
        Serial.flush();
        break;
    
    case NOT_OKAY:
        Serial.write(254);
        Serial.flush();
        break;

    default:
        break;
    }
}

unsigned long int TwoBytesToSteps(byte Byte1, byte Byte2, unsigned long int maxPosition)
{
    unsigned long int ValueTwoBytes = Byte1;
    ValueTwoBytes = ValueTwoBytes * 256;
    ValueTwoBytes = ValueTwoBytes + Byte2;
    if (maxPosition != 0){
        ValueTwoBytes = ValueTwoBytes * (maxPosition - 1);
        ValueTwoBytes = ValueTwoBytes / 65535;
        if (ValueTwoBytes > maxPosition - 1)
        {
            ValueTwoBytes = maxPosition - 1;
        }
    }
    return ValueTwoBytes;
}

void unsignedLongToTwoBytes(unsigned long int Value, unsigned long int MaxValue, byte* Byte1, byte* Byte2){
    double ValueScaled;
    if (MaxValue == 0){
        ValueScaled = Value;
    }else{
        ValueScaled = Value * 65535 / MaxValue;
    }
    unsigned int ValueScaledINT = (unsigned int) (ValueScaled);
    *Byte1 = ValueScaledINT / 256;
    *Byte2 = ValueScaledINT;
}


bool verifyData()
{
    byte veryfyingResult = 0;
    int x;
    for (x = 0; x != PROTOCOL_LENGTH -1; ++x)
    {
        veryfyingResult = veryfyingResult xor buffer[x];
    }
    if (veryfyingResult == buffer[PROTOCOL_LENGTH - 1])
    {
        return true;
    }
    else
    {
        return false;
    }
}


void sendBuffer(){
    buffer[PROTOCOL_LENGTH-1] = 0;
    int x;
    for (x = 0; x != PROTOCOL_LENGTH -1; ++x)
    {
        buffer[PROTOCOL_LENGTH-1] = buffer[PROTOCOL_LENGTH-1] xor buffer[x];
    }

    for (x = 0; x != PROTOCOL_LENGTH; ++x)
    {
        Serial.write(buffer[x]);
        Serial.flush();
    }
    waitingForOKAY = true;
    millisAtSendingAnswer = millis();
}


void sendValueRequest(){
    memset(buffer, 0, PROTOCOL_LENGTH);
    buffer[0] = int(unknownValue) * 2 + 1;
    sendBuffer();
    switch (unknownValue)
    {
    case HOMING_OFFSET:
        unknownValue = MAX_POSITION;
        break;

    case MAX_POSITION:
        unknownValue = ACCELLERATION;
        break;

    case ACCELLERATION:
        unknownValue = MAX_SPEED;
        break;

    case MAX_SPEED:
        unknownValue = IDLE;
        break;

    default:
        unknownValue = IDLE;
        break;
    }
}
