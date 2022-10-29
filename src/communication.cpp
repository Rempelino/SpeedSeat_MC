#include "communication.h"

communication::communication()
{
    for (size_t i = 0; i < sizeof request_buffer / sizeof request_buffer[0]; i++)
    {
        request_buffer[i] = IDLE;
    }
    request = IDLE;
}

void communication::execute()
{
    // communication is in IDLE and a Value request has been set
    if ((request_buffer[0] != IDLE) & (Serial.available() == 0) & !waiting_for_okay)
    {
        sendValueRequest();
        waiting_for_okay = true;
    }

    // waiting for acknowledgement of previous send command
    if (waiting_for_okay & ((Serial.available() == 1) || (Serial.available() == PROTOCOL_LENGTH + 1)))
    {
        if (Serial.read() == 255)
        {
            waiting_for_okay = false;
            int x = 0;
            while (request_buffer[x] != IDLE)
            {
                request_buffer[x] = request_buffer[x + 1];
                x++;
            }
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
    if (!waiting_for_okay && Serial.available() == PROTOCOL_LENGTH && request == IDLE && !recived_value.is_available)
    {
        readNewCommand();
    }

    // timeout while waiting for okay
    if (waiting_for_okay & TIMEOUT_ACTIVE)
    {
        if (millis() - millisAtLastSendMessage > TIMEOUT)
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

bool communication::verifyData(unsigned short buffer[PROTOCOL_LENGTH])
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
    unsigned short recivedData[PROTOCOL_LENGTH];
    int i;
    for (i = 0; i < PROTOCOL_LENGTH; i++)
    {
        recivedData[i] = Serial.read();
    }

    if (!verifyData(recivedData))
    {
        acknowledge(NOT_OKAY);
        return;
    }

    bool successfulExecuted;
    CMD command = (CMD)(recivedData[0] / 2);
    bool reading = (bool)(recivedData[0] % 2);
    switch (command)
    {
    case POSITION:
    case HOMING_OFFSET:
    case MAX_POSITION:
    case ACCELLERATION:
    case MAX_SPEED:
    case HOMING_STATUS:
    case NEW_HOMING:
    case HOMING_SPEED:
    case HOMING_ACCELERATION:
        if (reading)
        {
            request = command;
        }
        else
        {
            recived_value.as_int16[0] = recivedData[1] * 256 + recivedData[2];
            // recived_value.as_int16[0] = buffer[1] << 8 + buffer[2];
            recived_value.as_int16[1] = recivedData[3] * 256 + recivedData[4];
            recived_value.as_int16[2] = recivedData[5] * 256 + recivedData[6];
            recived_value.as_bool[0] = (bool)(recivedData)[1];
            recived_value.as_bool[1] = (bool)(recivedData)[3];
            recived_value.as_bool[2] = (bool)(recivedData)[5];
            recived_value.command = command;
            recived_value.is_available = true;
            successfulExecuted = true;
        }
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

void communication::unsignedLongToTwoBytes(unsigned long Value, unsigned long MaxValue, byte *Byte1, byte *Byte2)
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
    byte veryfyingResult = 0;
    for (int x = 0; x != PROTOCOL_LENGTH - 1; ++x)
    {
        veryfyingResult = veryfyingResult xor buffer[x];
    }
    buffer[PROTOCOL_LENGTH - 1] = veryfyingResult;

    for (size_t i = 0; i < PROTOCOL_LENGTH; i++)
    {
        Serial.write(buffer[i]);
        Serial.flush();
    }
    millisAtLastSendMessage = millis();
}

void communication::sendAnswer()
{
}

void communication::sendValueRequest()
{
    unsigned short commandByte = (unsigned short)(request_buffer[0]);
    commandByte = commandByte * 2 + 1;
    memset(buffer, 0, sizeof buffer);
    buffer[0] = commandByte;
    sendBuffer();
}

void communication::setNextValue()
{
}

void communication::get_value(CMD requestedValue)
{
    int x = 0;
    while (true)
    {
        if (request_buffer[x] == IDLE)
        {
            request_buffer[x] = requestedValue;
            return;
        }
        if (request_buffer[x] == requestedValue)
        {
            return;
        }
        x++;
    }
}

void communication::sendInitFinishedCommand()
{
    unsigned short commandByte = 0x02;
    memset(buffer, 0, sizeof buffer);
    buffer[0] = commandByte;
    buffer[PROTOCOL_LENGTH - 1] = commandByte;
    sendBuffer();
}

void communication::sendValue(CMD command, unsigned value1, unsigned value2, unsigned value3)
{
    unsigned short commandByte = (unsigned short)(command);
    commandByte = commandByte * 2;
    memset(buffer, 0, sizeof buffer);
    buffer[1] = 0;
    buffer[2] = 100;
    buffer[3] = 0;
    buffer[4] = 200;
    buffer[5] = 0;
    buffer[6] = 220;
    sendBuffer();
}