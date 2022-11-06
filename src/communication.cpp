#include "communication.h"
#include "configuration.h"

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
    if (waiting_for_okay && bytesRecived >= 1)
    {
        // check if first or last byte is OKAY
        if (recived_buffer[0] == 0xFF || (bytesRecived == PROTOCOL_LENGTH + 1 && recived_buffer[PROTOCOL_LENGTH] == 0xFF))
        {
            waiting_for_okay = false;
            valuesHavBeenFilled = false;
            bytesRecived--;
            int x = 0;

            // if more value request are queued shift them to position zero to let the main loop fill them with fitting values
            while (request_buffer[x] != IDLE)
            {
                request_buffer[x] = request_buffer[x + 1];
                x++;
            }

            // shift the recived buffer one slot to delete the okay and retain the command. Do not do that if okay(0xFF) was at the end of the buffer.
            if (recived_buffer[0] == 0xFF)
            {
                for (int i = 0; i != PROTOCOL_LENGTH; i++)
                {
                    recived_buffer[1] = recived_buffer[i + 1];
                }
            }
        }
        else if (recived_buffer[0] == 245 || (bytesRecived == PROTOCOL_LENGTH + 1 && recived_buffer[PROTOCOL_LENGTH] == 245))
        {
            sendBuffer();
        }
    }

    // if notOkay(0xFE) was recived without waiting for an okay just delete that byte
    if (bytesRecived > 0 && recived_buffer[0] == 0xFE)
    {
        bytesRecived--;
        for (int i = 0; i != PROTOCOL_LENGTH; i++)
        {
            recived_buffer[1] = recived_buffer[i + 1];
        }
    }

    // sending Value of requested command
    if (!waiting_for_okay && bytesRecived == 0 && request_buffer[0] != IDLE && valuesHavBeenFilled)
    {
        sendValue(request_buffer[0], valuesToSend[0], valuesToSend[1], valuesToSend[2]);
    }

    // reading new command
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
        acknowledge(NOT_OKAY);
    }

    // timeout in normal operation -> incomplete protocoll has been send
    if (bytesRecived != 0 && TIMEOUT_ACTIVE)
    {

        if (millis() - millisSinceBufferWasEmpty > TIMEOUT)
        {
            acknowledge(NOT_OKAY);
        }
    }
    else
    {
        millisSinceBufferWasEmpty = millis();
    }

#ifdef PUSH_STATUS_WHEN_COM_IN_IDLE
    if (bytesRecived == 0)
    {
        if (millis() - millisSinceBufferWasNotEmpty > 1000)
        {
            addCommandToRequestLine(IST_POSITION);
            addCommandToRequestLine(HOMING_STATUS);
            addCommandToRequestLine(FPS);
            millisSinceBufferWasNotEmpty = millis(); // TOBI delete this line to create maximum SPAM
        }
    }
    else
    {
        millisSinceBufferWasNotEmpty = millis();
    }
#endif
}

void communication::acknowledge(ANSWER answer)
{

    bytesRecived = 0;
    millisSinceBufferWasEmpty = millis();
    millisSinceBufferWasNotEmpty = millis();
    switch (answer)
    {
    case OKAY:
        Serial.write(0xFF);
        Serial.flush();
        break;

    case NOT_OKAY:
        failedCommands++;
        delay(20); // wait for possible transmission to end
        while (Serial.available() != 0)
        {
            Serial.read();
        }
        Serial.write(0xFE);
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
        acknowledge(NOT_OKAY);
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
        if (ringCounter == 40)
        {
            ringCounter = 0;
        }
        if (ringCounter == 0)
        {
            calculateCycleTime();
        }
    }
    else
    {
        acknowledge(NOT_OKAY);
    }
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
    addCommandToRequestLine(IST_POSITION);
    addCommandToRequestLine(MAX_POSITION);
    addCommandToRequestLine(HOMING_OFFSET);
    addCommandToRequestLine(ACCELLERATION);
    addCommandToRequestLine(MAX_SPEED);
    addCommandToRequestLine(HOMING_STATUS);
    addCommandToRequestLine(HOMING_SPEED);
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
    fps = (unsigned)(40.0 / (timeFor256Commands / 1000.0));
    addCommandToRequestLine(FPS);
}

void communication::addDataToRecivedBuffer()
{
    if (bytesRecived == PROTOCOL_LENGTH + 1)
    {
        acknowledge(NOT_OKAY);
        return;
    }
    recived_buffer[bytesRecived] = Serial.read();
    bytesRecived++;
}