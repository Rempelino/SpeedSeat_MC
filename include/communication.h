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

    IDLE = 9
};

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
    unsigned long scaled_to_max_axis_pos_as_steps[3];
    unsigned long int scaled_to_steps[3];
    unsigned long int as_steps[3];
    CMD command;
};

class communication
{
    //byte buffer[PROTOCOL_LENGTH];
    unsigned short buffer[PROTOCOL_LENGTH];
    unsigned long int millis_at_sending_answer;
    unsigned long int steps_per_millimeter;
    unsigned long int axis_max_position_as_steps[3];
    bool waiting_for_okay;
    CMD request;
    CMD request_buffer[100];
    void acknowledge(ANSWER);
    bool verifyData();
    void readNewCommand();
    void unsignedLongToTwoBytes(unsigned long int, unsigned long int, byte *, byte *);
    void sendBuffer();
    void sendAnswer();
    void sendValueRequest();
    void setNextValue();

public:
    void initialize(unsigned long int Steps_per_millimeter,
                    unsigned long int Axis_1_max_position,
                    unsigned long int Axis_2_max_position,
                    unsigned long int Axis_3_max_position);
        void get_value(CMD);
        void execute();
        unsigned long int TwoBytesToSteps(byte, byte, unsigned long int);
        AvailableInfos recived_value;
        CMD available_command;
    };

#endif