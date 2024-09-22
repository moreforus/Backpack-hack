#include <Arduino.h>
#include <Terrestrial/remoteConsole.h>

RemoteConsole::RemoteConsole(unsigned long baud)
{
    Serial.begin(baud);
    Serial.setTimeout(1);
}

void
RemoteConsole::Init()
{

}

std::string
RemoteConsole::MakeMessage(const TerrestrialResponse_t& response)
{
    uint64_t us = micros();
    std::string str(response.work == WORK_MODE_TYPE::RECEIVER ? "R" : "S");
    str += ":" + std::to_string(response.freq);
    str += "[" + std::to_string(response.rssiA);
    str += ":" + std::to_string(response.rssiB);
    str += "]" + std::to_string(response.antenna);
    str += ":" + std::to_string(us);
    str += "\r\n";
    
    return str;
}

TerrestrialCommand_t
RemoteConsole::ParseCommand(const std::string& command)
{
    TerrestrialCommand_t terrCommand;
    terrCommand.work = WORK_MODE_TYPE::NONE;

    // Rxxxx\n -- set receiving on xxxx freq
    // Sxxxx:yyyy:dddd:ii\n -- set scanner mode from [xxxx] to [yyyy] with step [ii] MHz freq with delay [dddd] ms on each freq
    if (!command.empty())
    {
        if (command[0] == 'R')
        {
            terrCommand.work = WORK_MODE_TYPE::RECEIVER;
            terrCommand.freq = atof(command.c_str() + 1);
        }
        else if (command[0] == 'S')
        {
            char tmp[25];
            auto buffer = command.c_str();

            // get xxxx:
            auto separatorPos = command.find(':');
            strncpy(tmp, buffer + 1, separatorPos - 1);
            tmp[separatorPos - 1] = 0;
            frequency_t minFreq = atof(tmp);

            // get :yyyy:
            auto separatorPos1 = command.find(':', separatorPos + 1);
            strncpy(tmp, buffer + separatorPos + 1, separatorPos1 - separatorPos - 1);
            tmp[separatorPos1 - separatorPos - 1] = 0;
            frequency_t maxFreq = atof(tmp);
            
            // get :dddd:
            separatorPos = command.find(':', separatorPos1 + 1);
            strncpy(tmp, buffer + separatorPos + 1, separatorPos - separatorPos1 - 1);
            tmp[separatorPos - separatorPos1 - 1] = 0;
            terrCommand.scannerFilter = atoi(tmp);

            // get :ii
            strcpy(tmp, buffer + separatorPos + 1);
            tmp[strlen(buffer) - separatorPos - 1] = 0;
            terrCommand.scannerStep  = atof(tmp);

            if (minFreq > maxFreq)
            {
                std::swap(minFreq, maxFreq);
            }

            terrCommand.work = WORK_MODE_TYPE::SCANNER;
            terrCommand.scannerFrom = minFreq;
            terrCommand.scannerTo = maxFreq;
        }
    }
    
    return terrCommand;
}

void
RemoteConsole::Loop()
{
    if (Serial.availableForWrite())
    {
        auto response = GetMessage();
        if (response.work != WORK_MODE_TYPE::NONE)
        {
            auto str = MakeMessage(response);
            Serial.print(str.c_str());
        }
    }

    auto data = Serial.read();
    if (data > 0)
    {
        if ((data == '\n' || data == '\r'))
        {
            if (_pointer > 0)
            {
                _buffer[_pointer] = 0;
                _command = ParseCommand(_buffer);
            }

            _pointer = 0;
            return;
        }

        _buffer[_pointer] = data;
        ++_pointer;
    }
}

