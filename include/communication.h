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

#ifndef STRUCT_CMD
#define STRUCT_CMD
enum CMD
{
    POSITION = 0,
    MAX_POSITION = 1,
    HOMING_OFFSET = 2,
    ACCELLERATION = 3,
    MAX_SPEED = 4,
    HOMING_STATUS = 5,
    NEW_HOMING = 6,
    HOMING_SPEED = 7,
    HOMING_ACCELERATION = 8,

    IDLE = 9
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
    unsigned long millisAtLastSendMessage;
    unsigned long steps_per_millimeter;
    unsigned long axis_max_position_as_steps[3];
    bool waiting_for_okay;
    CMD request;
    CMD request_buffer[100];
    bool verifyData(unsigned short buffer[PROTOCOL_LENGTH]);
    void readNewCommand();
    void unsignedLongToTwoBytes(unsigned long, unsigned long, byte *, byte *);
    void sendBuffer();
    void sendAnswer();
    void sendValueRequest();
    void setNextValue();

public:
    void acknowledge(ANSWER);
    void sendValue(CMD command, unsigned value1, unsigned value2 ,unsigned value3);
    communication();
    void get_value(CMD);
    void execute();
    void sendInitFinishedCommand();
    unsigned long TwoBytesToSteps(byte, byte, unsigned long);
    AvailableInfos recived_value;
    CMD available_command;
};
#endif