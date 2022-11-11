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
                    recived_buffer[i] = recived_buffer[i + 1];
                }
            }
        }
        else if (recived_buffer[0] == 0xFE || (bytesRecived == PROTOCOL_LENGTH + 1 && recived_buffer[PROTOCOL_LENGTH] == 0xFE))
        {
            failedCommands++;
            sendBuffer();
        }
    }

    // if notOkay(0xFE) was recived without waiting for an okay just delete that byte
    if (bytesRecived > 0 && recived_buffer[0] == 0xFE)
    {
        failedCommands++;
        bytesRecived--;
        for (int i = 0; i != PROTOCOL_LENGTH; i++)
        {
            recived_buffer[i] = recived_buffer[i + 1];
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

    if (bytesRecived == 0 && sendState)
    {
        if (millis() - millisLastStateUpdate > stateUpdateIntervall)
        {
            addCommandToRequestLine(IST_POSITION);
            addCommandToRequestLine(HOMING_STATUS);
            addCommandToRequestLine(INFORMATION);
            millisLastStateUpdate = millis();
        }
    }
    calculateCycleTime();
}

void communication::acknowledge(ANSWER answer)
{

    bytesRecived = 0;
    millisSinceBufferWasEmpty = millis();
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
        /*for (int i = 0; i != PROTOCOL_LENGTH; i++)
        {
            Serial.write(recived_buffer[i]);
            Serial.flush();
        }

        while (1)
        {
        }*/
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
    case SAVE_SETTINGS:
    case RESET_EEPROM:
        if (reading)
        {
            addCommandToRequestLine(command);
        }
        else
        {
            recived_value.as_int16[0] = (recived_buffer[1] << 8) | recived_buffer[2];
            recived_value.as_int16[1] = (recived_buffer[3] << 8) | recived_buffer[4];
            recived_value.as_int16[2] = (recived_buffer[5] << 8) | recived_buffer[6];
            recived_value.as_bool[0] = (bool)(recived_buffer)[2];
            recived_value.as_bool[1] = (bool)(recived_buffer)[4];
            recived_value.as_bool[2] = (bool)(recived_buffer)[6];
            recived_value.command = command;
            recived_value.is_available = true;
        }
        successfulExecuted = true;
        break;

    case INIT_REQUEST:
        addAllCommandsToRequestLine();
        successfulExecuted = true;
        break;

    case STATE_UPDATE_INTERVALL:
        stateUpdateIntervall = recived_buffer[1] * 256 + recived_buffer[2];
        sendState = (bool)(recived_buffer)[4];
        successfulExecuted = true;
        break;

    default:
        successfulExecuted = false;
        break;
    }

    if (successfulExecuted)
    {
        acknowledge(OKAY);
        commandCounter++;
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
    addCommandToRequestLine(INFORMATION);
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
    const unsigned long calculationIntervall = 1000;
    if (millis() - millisAtLastFPSCalculation > calculationIntervall)
    {
        millisAtLastFPSCalculation += calculationIntervall;
        fps = commandCounter;
        commandCounter = 0;
    }
}

void communication::addDataToRecivedBuffer()
{
    if (bytesRecived == PROTOCOL_LENGTH + 1)
    {
        acknowledge(NOT_OKAY);
        return;
    }

    unsigned short c = Serial.read();
    if (c <= 0xFF)
    {
        recived_buffer[bytesRecived] = c;
        bytesRecived++;
    }
}