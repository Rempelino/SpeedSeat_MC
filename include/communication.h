#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "Arduino.h"
#ifndef PROTOCOL_LENGTH
#define PROTOCOL_LENGTH 8
#endif

#ifndef TIMEOUT
#define TIMEOUT 1000
#endif

#ifndef TIMEOUT_ACTIVE
#define TIMEOUT_ACTIVE true
#endif

#ifndef REQUEST_BUFFER_LENGTH
#define REQUEST_BUFFER_LENGTH 100
#endif

#ifndef STRUCT_CMD
#define STRUCT_CMD
enum CMD
{
    POSITION = 0,
    INIT_REQUEST = 1,
    INIT_SUCCESSFUL = 2,
    MAX_POSITION = 3,
    HOMING_OFFSET = 4,
    ACCELLERATION = 5,
    MAX_SPEED = 6,
    HOMING_STATUS = 7,
    NEW_HOMING = 8,
    HOMING_SPEED = 9,
    HOMING_ACCELERATION = 10,
    FPS = 11,

    IDLE = 999
};
#endif
enum ANSWER
{
    OKAY = 0,
    NOT_OKAY = 1,
    NO_ANSWER = 3
};

struct AvailableInfos
{
    bool is_available;
    unsigned int as_int16[3];
    bool as_bool[3];
    CMD command;
};

class communication
{
    unsigned short buffer[PROTOCOL_LENGTH];
    unsigned short recived_buffer[PROTOCOL_LENGTH + 1];
    int bytesRecived;
    unsigned long millisAtLastSendMessage;
    unsigned long millisSinceBufferWasEmpty;
    unsigned long millisSinceBufferWasNotEmpty;
    unsigned long steps_per_millimeter;
    unsigned long axis_max_position_as_steps[3];

    unsigned long cycleTime;
    uint8_t ringCounter;

    bool waiting_for_okay;
    bool valuesHavBeenFilled = false;
    bool hasBeendInitialized = false;
    unsigned valuesToSend[3];

    const int requestBufferLength = 100;
    CMD request_buffer[REQUEST_BUFFER_LENGTH];
    bool verifyData();
    void readNewCommand();
    void unsignedLongToTwoBytes(unsigned long, unsigned long, byte *, byte *);
    void sendBuffer();
    void sendAnswer();
    void sendValueRequest();
    void setNextValue();
    void addCommandToRequestLine(CMD);
    void addAllCommandsToRequestLine();
    void calculateCycleTime();
    void addDataToRecivedBuffer();

public:
    void acknowledge(ANSWER);void acknowledge(ANSWER, int);
    void sendValue(CMD command, unsigned value1, unsigned value2 ,unsigned value3);
    communication();
    void get_value(CMD);
    void execute();
    void fillValueBuffer(unsigned Value1, unsigned Value2, unsigned Value3);
    unsigned fps;
    CMD getRequestedValue();
    unsigned long TwoBytesToSteps(byte, byte, unsigned long);
    AvailableInfos recived_value;
    CMD available_command;
};
#endif