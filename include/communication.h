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

enum CMD
{
    POSITION = 0,
    HOMING_OFFSET = 1,
    MAX_POSITION = 2,
    ACCELLERATION = 3,
    MAX_SPEED = 6,
    HOMING_STATUS = 7,
    STEPS_PER_MILLIMETER = 8,
    IDLE = 9
};

enum ANSWER
{
    OKAY = 0,
    NOT_OKAY = 1,
    NO_ANSWER = 3
};

struct AvailableInfos{
    bool is_available;
    unsigned int value_int_1;
    unsigned int value_int_2;
    unsigned int value_int_3;
    CMD command;
    bool value_bool;
};

class communication
{
    byte buffer[PROTOCOL_LENGTH];
    unsigned long millis_at_sending_answer;
    bool waiting_for_okay;
    CMD request;
    CMD request_buffer[100];

    void acknowledge(ANSWER);
    unsigned long int TwoBytesToSteps(byte, byte, unsigned long int);
    bool verifyData();
    void readNewCommand();
    void unsignedLongToTwoBytes(unsigned long int, unsigned long int, byte *, byte *);
    void sendBuffer();
    void sendAnswer();
    void sendValueRequest();
    void setNextValue();

public:
    void initialize();
    void get_value(CMD);
    void execute();
    AvailableInfos info;
    CMD available_command;
};

#endif