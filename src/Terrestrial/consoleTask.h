#pragma once
#include <Arduino.h>
#include <string>
#include <Terrestrial/remoteConsole.h>
#include <Terrestrial/userConsole.h>
#include <Terrestrial/workMode.h>

QueueHandle_t commandQueue = nullptr;
QueueHandle_t responseQueue = nullptr;

static TerrestrialCommand_t
ParseCommand(const std::string& command)
{
    TerrestrialCommand_t terrCommand;
    terrCommand.work = WORK_MODE_TYPE::NONE;

    // Rxxxx\n -- set receiving on xxxx freq
    // Sxxxx:yyyy:dddd:ii\n -- set scanner mode from [xxxx] to [yyyy] with step [ii] MHz freq with delay [dddd] ms on each freq
    if (!command.empty())
    {
        if (command[0] == 'R')
        {
            if (command.size() >= 4)
            {
                terrCommand.work = WORK_MODE_TYPE::RECEIVER;
                terrCommand.freq = atoi(command.c_str() + 1);
            }
        }
        else if (command[0] == 'S')
        {
            if (command.size() >= 17)
            {
                char tmp[5];
                auto buffer = command.c_str();
                strncpy(tmp, buffer + 1, 4);
                tmp[4] = 0;
                uint16_t minFreq = atoi(tmp);
                strncpy(tmp, buffer + 6, 4);
                tmp[4] = 0;
                uint16_t maxFreq = atoi(tmp);
                strncpy(tmp, buffer + 11, 4);
                tmp[4] = 0;
                terrCommand.scannerFilter = atoi(tmp);
                strncpy(tmp, buffer + 16, 2);
                tmp[2] = 0;
                terrCommand.scannerStep  = atoi(tmp);

                if (minFreq > maxFreq)
                {
                    std::swap(minFreq, maxFreq);
                }

                terrCommand.work = WORK_MODE_TYPE::SCANNER;
                terrCommand.scannerFrom = minFreq;
                terrCommand.scannerTo = maxFreq;
            }
        }
    }
    
    return terrCommand;
}

static std::string
MakeMessage(const TerrestrialResponse_t& response)
{
    uint64_t us = micros();
    char str[48];
    sprintf(str, "%s:%d[%d:%d]%d>%llu\r\n", response.work == WORK_MODE_TYPE::RECEIVER ? "R" : "S"
                                          , response.freq, response.rssiA, response.rssiB, response.antenna, us);
    
    return str;
}

void consoleTask(void* params)
{
    IConsole* _remoteConsole;
    IConsole* _userConsole;

    commandQueue = xQueueCreate(10, sizeof(TerrestrialCommand_t));
    responseQueue = xQueueCreate(10, sizeof(TerrestrialResponse_t));

    TERRESTRIAL_STATE* _state = (TERRESTRIAL_STATE*)params;
    _remoteConsole = new RemoteConsole(921600);
    _remoteConsole->Init();

    _userConsole = new UserConsole(_state);
    _userConsole->Init();

    uint32_t currentTimeMs = 0;

    for (;;)
    {
        auto usStart = micros();
        uint32_t now = millis();
        _remoteConsole->Loop();
        auto command = _remoteConsole->GetCommand();
        if (!command.empty())
        {
            auto mode = ParseCommand(command);
            xQueueSend(commandQueue, (void *) &mode, (TickType_t)0);
        }

        TerrestrialResponse_t response;
        if (xQueueReceive(responseQueue, &response, 0))
        {
            auto message = MakeMessage(response);
            _remoteConsole->SendMessage(message);
            _userConsole->SendMessage(message);
        }
    
        if (currentTimeMs == now)
        {
            continue;
        }

        currentTimeMs = now;
        auto usI2CStart = micros();
        _userConsole->Loop();
        command = _userConsole->GetCommand();
        if (!command.empty())
        {
            auto mode = ParseCommand(command);
            xQueueSend(commandQueue, (void *) &mode, (TickType_t)0);
        }

        auto usStop = micros();
        uint8_t i2c = (usStop - usI2CStart) / 10;
        _state->device.i2c = i2c > 100 ? 100 : i2c;
        uint8_t cpu = (usStop - usStart) / 10;
        _state->device.cpu0 = cpu > 100 ? 100 : cpu;
    }
}
