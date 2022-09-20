#include "communication.h"

void communication::initialize(unsigned long int Steps_per_millimeter,
                    unsigned long int Axis_1_max_position,
                    unsigned long int Axis_2_max_position,
                    unsigned long int Axis_3_max_position)
{
    axis_max_position_as_steps[0] = Axis_1_max_position * Steps_per_millimeter;
    axis_max_position_as_steps[1] = Axis_2_max_position * Steps_per_millimeter;
    axis_max_position_as_steps[2] = Axis_3_max_position * Steps_per_millimeter;
    request_buffer[0] = IDLE;
    request = IDLE;
}

void communication::execute()
{
    // communication is in IDLE and a Value request has been set
    if ((request_buffer[0] != IDLE) & (Serial.available() == 0) & !waiting_for_okay)
    {
        sendValueRequest();
    }

    // waiting for acknowledgement of previous send command
    if (waiting_for_okay & ((Serial.available() == 1) || (Serial.available() == PROTOCOL_LENGTH + 1)))
    {
        if (Serial.read() == 255)
        {
            waiting_for_okay = false;
        }
        else
        {
            acknowledge(NO_ANSWER);
            sendBuffer();
        }
    }

    // sending Value of requested command
    if (!waiting_for_okay & (request != IDLE))
    {
        sendAnswer();
    }

    // reading new command
    if (!waiting_for_okay & (Serial.available() == PROTOCOL_LENGTH) & (request == IDLE))
    {
        readNewCommand();
    }

    // timeout while waiting for okay
    if (waiting_for_okay & TIMEOUT_ACTIVE)
    {
        if (millis() - millis_at_sending_answer > TIMEOUT)
        {
            sendBuffer();
        }
    }

    // buffer pverflow -> reseting communication
    if ((Serial.available() > PROTOCOL_LENGTH + (int)(waiting_for_okay)))
    {
        acknowledge(NOT_OKAY);
    }
}

void communication::acknowledge(ANSWER answer)
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

bool communication::verifyData()
{
    byte veryfyingResult = 0;
    int x;
    for (x = 0; x != PROTOCOL_LENGTH - 1; ++x)
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

void communication::readNewCommand()
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

    bool successfulExecuted;
    CMD command = (CMD)(buffer[0] / 2);
    bool reading = (bool)(buffer[0] % 2);
    switch (command)
    {
    case POSITION: case HOMING_OFFSET: case MAX_POSITION: case ACCELLERATION: case MAX_SPEED: case HOMING_STATUS:
        if (reading)
        {
            request = command;
        }
        else
        {
            recived_value.as_int16[0] = buffer[1] * 256 + buffer[2];
            recived_value.as_int16[1] = buffer[3] * 256 + buffer[4];
            recived_value.as_int16[2] = buffer[5] * 256 + buffer[6];
            recived_value.as_bool[0] = (bool)(buffer)[1];
            recived_value.as_bool[1] = (bool)(buffer)[3];
            recived_value.as_bool[2] = (bool)(buffer)[5];
            if (axis_max_position_as_steps[0] != 0){
                recived_value.scaled_to_max_axis_pos_as_steps[0] = recived_value.as_int16[0] * axis_max_position_as_steps[0] / 65535;
            }
            if (axis_max_position_as_steps[1] != 0){
                recived_value.scaled_to_max_axis_pos_as_steps[1] = recived_value.as_int16[1] * axis_max_position_as_steps[1] / 65535;
            }
            if (axis_max_position_as_steps[2] != 0){
                recived_value.scaled_to_max_axis_pos_as_steps[2] = recived_value.as_int16[2] * axis_max_position_as_steps[2] / 65535;
            }
            recived_value.as_steps[0] = recived_value.as_int16[0] * steps_per_millimeter;
            recived_value.as_steps[1] = recived_value.as_int16[1] * steps_per_millimeter;
            recived_value.as_steps[2] = recived_value.as_int16[2] * steps_per_millimeter;

            recived_value.command = command;
            recived_value.is_available = true;
        }
        successfulExecuted = true;
        break;

    default:
        successfulExecuted = false;
        break;
    }

    if (successfulExecuted)
    {
        acknowledge(OKAY);
        if (reading)
        {
            sendAnswer();
        }
    }
    else
    {
        acknowledge(NOT_OKAY);
    }
}

void unsignedLongToTwoBytes(unsigned long int Value, unsigned long int MaxValue, byte *Byte1, byte *Byte2)
{
    double ValueScaled;
    if (MaxValue == 0)
    {
        ValueScaled = Value;
    }
    else
    {
        ValueScaled = Value * 65535 / MaxValue;
    }
    unsigned int ValueScaledINT = (unsigned int)(ValueScaled);
    *Byte1 = ValueScaledINT / 256;
    *Byte2 = ValueScaledINT;
}

void communication::sendBuffer()
{
}

void communication::sendAnswer()
{
}

void communication::sendValueRequest()
{
}

void communication::setNextValue()
{
}

void communication::get_value(CMD)
{
}
