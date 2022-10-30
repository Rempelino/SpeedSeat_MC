#include "communication.h"

communication::communication()
{
    for (size_t i = 0; i < sizeof request_buffer / sizeof request_buffer[0]; i++)
    {
        request_buffer[i] = IDLE;
    }
}

void communication::execute()
{
    while (Serial.available() != 0)
    {
        addDataToRecivedBuffer();
    }

    // waiting for acknowledgement of previous send command
    if (waiting_for_okay & (bytesRecived == 1) || (bytesRecived == PROTOCOL_LENGTH + 1))
    {
        //check if first or last byte is OKAY
        if (recived_buffer[0] == 255 || (bytesRecived == PROTOCOL_LENGTH + 1 && recived_buffer[PROTOCOL_LENGTH] == 255))
        {
            waiting_for_okay = false;
            valuesHavBeenFilled = false;
            int x = 0;
            while (request_buffer[x] != IDLE)
            {
                request_buffer[x] = request_buffer[x + 1];
                x++;
            }
            if (bytesRecived == 1)
            {
                bytesRecived = 0;
            }
            else
            {
                bytesRecived--;
                for (int i = 0; i < PROTOCOL_LENGTH; i++)
                {
                    recived_buffer[1] = recived_buffer[i + 1];
                }
            }
        }
        else if(recived_buffer[0] == 245 || (bytesRecived == PROTOCOL_LENGTH + 1 && recived_buffer[PROTOCOL_LENGTH] == 245))
        {
            sendBuffer();
        }
    }

    // sending Value of requested command
    if (!waiting_for_okay && bytesRecived == 0 && request_buffer[0] != IDLE && valuesHavBeenFilled)
    {
        sendValue(request_buffer[0], valuesToSend[0], valuesToSend[1], valuesToSend[2]);
    }

    // reading new command
    // if (!waiting_for_okay && Serial.available() == PROTOCOL_LENGTH && !recived_value.is_available)
    if (bytesRecived == PROTOCOL_LENGTH && !recived_value.is_available)
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

    // buffer overflow -> reseting communication
    if ((bytesRecived > PROTOCOL_LENGTH + (int)(waiting_for_okay)))
    {
        acknowledge(NOT_OKAY, 1);
    }

    // timeout in normal operation -> incomplete protocoll has been send
    if (bytesRecived != 0 && TIMEOUT_ACTIVE)
    {

        if (millis() - millisSinceBufferWasEmpty > TIMEOUT)
        {
            acknowledge(NOT_OKAY, 2);
        }
    }
    else
    {
        millisSinceBufferWasEmpty = millis();
    }

    // TOBI you might comment to stop spam
    // send status when communication is not busy
    if (bytesRecived == 0)
    {
        if (millis() - millisSinceBufferWasNotEmpty > 1000)
        {
            //addCommandToRequestLine(POSITION);
            //addCommandToRequestLine(HOMING_STATUS);
            millisSinceBufferWasNotEmpty = millis(); //TOBI delete this line to create maximum SPAM
        }
    }
    else
    {
        millisSinceBufferWasNotEmpty = millis();
    }
    //-----------------------------
}

void communication::acknowledge(ANSWER answer)
{

    bytesRecived = 0;
    millisSinceBufferWasEmpty = millis();
    millisSinceBufferWasNotEmpty = millis();
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

void communication::acknowledge(ANSWER answer, int offset)
{

    bytesRecived = 0;
    millisSinceBufferWasEmpty = millis();
    millisSinceBufferWasNotEmpty = millis();
    switch (answer)
    {
    case OKAY:
        Serial.write(255);
        Serial.flush();
        break;

    case NOT_OKAY:
        Serial.write(254 - offset);
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
        veryfyingResult = veryfyingResult xor recived_buffer[x];
    }
    if (veryfyingResult == recived_buffer[PROTOCOL_LENGTH - 1])
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
    if (!verifyData())
    {
        for (int i = 0; i < PROTOCOL_LENGTH; i++)
        {
            Serial.write(recived_buffer[i]);
            Serial.flush();
        }

        acknowledge(NOT_OKAY, 3);
        return;
    }

    bool successfulExecuted;
    CMD command = (CMD)(recived_buffer[0] / 2);
    bool reading = (bool)(recived_buffer[0] % 2);

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
            addCommandToRequestLine(command);
        }
        else
        {
            recived_value.as_int16[0] = recived_buffer[1] * 256 + recived_buffer[2];
            recived_value.as_int16[1] = recived_buffer[3] * 256 + recived_buffer[4];
            recived_value.as_int16[2] = recived_buffer[5] * 256 + recived_buffer[6];
            recived_value.as_bool[0] = (bool)(recived_buffer)[1];
            recived_value.as_bool[1] = (bool)(recived_buffer)[3];
            recived_value.as_bool[2] = (bool)(recived_buffer)[5];
            recived_value.command = command;
            recived_value.is_available = true;
        }
        successfulExecuted = true;
        break;

    case INIT_REQUEST:
        addAllCommandsToRequestLine();
        successfulExecuted = true;
        break;

    default:
        successfulExecuted = false;
        break;
    }

    if (successfulExecuted)
    {
        acknowledge(OKAY);
        ringCounter++;
        if (ringCounter == 0)
        {
            calculateCycleTime();
        }
    }
    else
    {
        acknowledge(NOT_OKAY, 4);
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
    waiting_for_okay = true;

    // DELETE THIS TO STOP IGNORING TOBIS BUGS
    if (buffer[0] == 4)
    {
        waiting_for_okay = false;
        valuesHavBeenFilled = false;
        int x = 0;
        while (request_buffer[x] != IDLE)
        {
            request_buffer[x] = request_buffer[x + 1];
            x++;
        }
        delay(500);
    }
    //------------------------------------
    millisAtLastSendMessage = millis();
}

void communication::sendValue(CMD command, unsigned value1, unsigned value2, unsigned value3)
{
    unsigned short commandByte = (unsigned short)(command);
    commandByte = commandByte * 2;
    memset(buffer, 0, sizeof buffer);
    buffer[0] = commandByte;
    buffer[1] = value1 >> 8;
    buffer[2] = value1;
    buffer[3] = value2 >> 8;
    buffer[4] = value2;
    buffer[5] = value3 >> 8;
    buffer[6] = value3;
    sendBuffer();
}

void communication::addAllCommandsToRequestLine()
{
    addCommandToRequestLine(POSITION);
    addCommandToRequestLine(MAX_POSITION);
    addCommandToRequestLine(HOMING_OFFSET);
    addCommandToRequestLine(ACCELLERATION);
    addCommandToRequestLine(MAX_SPEED);
    addCommandToRequestLine(HOMING_SPEED);
    addCommandToRequestLine(HOMING_STATUS);
    addCommandToRequestLine(HOMING_ACCELERATION);
    addCommandToRequestLine(FPS);
    addCommandToRequestLine(INIT_SUCCESSFUL);
}

void communication::addCommandToRequestLine(CMD command)
{
    for (int i = 0; i < REQUEST_BUFFER_LENGTH; i++)
    {
        if (request_buffer[i] == command)
        {
            return;
        }
        if (request_buffer[i] == IDLE)
        {
            request_buffer[i] = command;
            return;
        }
    }
}

CMD communication::getRequestedValue()
{
    return request_buffer[0];
}

void communication::fillValueBuffer(unsigned Value1, unsigned Value2, unsigned Value3)
{
    if (getRequestedValue() == IDLE)
    {
        return;
    }
    if (valuesHavBeenFilled)
    {
        return;
    }
    valuesToSend[0] = Value1;
    valuesToSend[1] = Value2;
    valuesToSend[2] = Value3;
    valuesHavBeenFilled = true;
}

void communication::calculateCycleTime()
{
    double timeFor256Commands = millis() - cycleTime;
    cycleTime = millis();

    if (timeFor256Commands == 0)
    {
        return;
    }
    fps = (unsigned)(256.0 / (timeFor256Commands / 1000.0));
    addCommandToRequestLine(FPS);
}

void communication::addDataToRecivedBuffer()
{
    if (bytesRecived == PROTOCOL_LENGTH + 1)
    {
        acknowledge(NOT_OKAY, 6);
        return;
    }
    recived_buffer[bytesRecived] = Serial.read();
    bytesRecived++;
}