#include "terrestrial.h"
#include <SPI.h>
#include "logging.h"
#include <rtc6715.h>
#include <lib_rtc6712.h>
#include <Terrestrial/remoteConsole.h>
#include <Terrestrial/userConsole.h>
#include <common.h>
#include "config.h"

#define RSSI_DIFF_BORDER (16)

#define MIN_1G2_FREQ (500)
#define MAX_1G2_FREQ (2500)

#define MIN_5G8_FREQ (4900)
#define MAX_5G8_FREQ (6000)

void
Terrestrial::Init()
{
    ModuleBase::Init();
    auto cfg = _config->GetTerrestrialConfig();
    _state.receiver.currentFreq = cfg->receiverFreq;
    _state.scanner.from = cfg->from;
    _state.scanner.to = cfg->to;
    _state.scanner.filter = cfg->filter;
    _state.scanner.step = cfg->step;

    // common MOSI and CLK pins
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_5G8_CS, INPUT);
    pinMode(PIN_1G2_CS, INPUT);
    pinMode(VIDEO_CTRL, OUTPUT);

    memset(_state.rssi, 0, sizeof(_state.rssi));

    DBGLN("Terrestrial init complete");
    _remoteConsole = new RemoteConsole(921600);
    _remoteConsole->Init();

    _userConsole = new UserConsole(&_state);
    _userConsole->Init();

    _scaner1G2 = new Scaner(rtc6712SetFreq, RSSI_1G2_A, RSSI_1G2_B, RSSI_DIFF_BORDER);
    _scaner5G8 = new Scaner(rtc6715SetFreq, RSSI_5G8_A, RSSI_5G8_B, RSSI_DIFF_BORDER);
}

void
Terrestrial::EnableSPIMode()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_5G8_CS, OUTPUT);
    pinMode(PIN_1G2_CS, OUTPUT);
    
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_5G8_CS, HIGH);
    digitalWrite(PIN_1G2_CS, HIGH);

    SPIModeEnabled = true;

    DBGLN("SPI config complete");
}

void
Terrestrial::SetFreq(uint16_t freq)
{
    if (freq >= MIN_5G8_FREQ)
    {
        rtc6715SetFreq(freq);
        _state.receiver.currentFreq = freq;
    }
    else if (freq < MAX_1G2_FREQ)
    {
        rtc6712SetFreq(freq);
        _state.receiver.currentFreq = freq;
    }
}

void
Terrestrial::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    _state.receiver.currentFreq = frequencyTable[index];
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    SetFreq(_state.receiver.currentFreq);
    SaveConfig();
}

void
Terrestrial::SetWorkMode(WORK_MODE_TYPE mode)
{
    if (mode == NONE)
    {
        return;
    }
    
    workMode = mode;
    if (workMode == SCANNER)
    {
        // restart scanning process.
        scannerAuto = INIT;
    }
    else if (workMode == RECEIVER)
    {
        if (!SPIModeEnabled) 
        {
            EnableSPIMode();
        }

        SetFreq(_state.receiver.currentFreq);
    }
}

std::string
Terrestrial::MakeMessage(const char* cmd, const uint16_t freq)
{
    uint64_t us = micros();
    char str[48];
    sprintf(str, "%s:%d[%d:%d]%d>%llu\r\n", cmd, freq, _state.rssiA, _state.rssiB, currentAntenna, us);
    //sprintf(str, "%s:%d[%d:%d]%d>0\r\n", cmd, freq, _state.rssiA, _state.rssiB, currentAntenna);
    
    return str;
}

#define DEFAULT_RECEIVER_FILTER (8)

std::string
Terrestrial::Work()
{
    std::string message;

    if (workMode == RECEIVER)
    {
        ANTENNA_TYPE antenna = ANT_A;
        if (CheckRSSI(antenna, DEFAULT_RECEIVER_FILTER))
        {
            if (antenna != currentAntenna)
            {
                currentAntenna = antenna;
                SwitchVideo(currentAntenna);
            }

            message = MakeMessage("R", _state.receiver.currentFreq);
        }
    }
    else if (workMode == SCANNER)
    {
        switch (scannerAuto)
        {
            case INIT:
                _scaner1G2->Init(minScaner1G2Freq, maxScaner1G2Freq);
                _scaner5G8->Init(minScaner5G8Freq, maxScaner5G8Freq);

                scannerAuto = SET_FREQ_1G2;
            break;
            case SET_FREQ_1G2:
                if (!SPIModeEnabled) 
                {
                    EnableSPIMode();
                }

                _scaner1G2->SetFreq(_scanerFilter);
                scannerAuto = SET_FREQ_5G8;
            break;

            case SET_FREQ_5G8:
                _scaner5G8->SetFreq(_scanerFilter);
                scannerAuto = MEASURE;
            break;

            case MEASURE:
                if (_scaner1G2->MeasureRSSI())
                {
                    message = _scaner1G2->MakeMessage();

                    uint16_t from = MIN_1G2_FREQ;
                    uint16_t to = MAX_1G2_FREQ;
                    if (_state.scanner.from < MAX_1G2_FREQ)
                    {
                        from = _state.scanner.from;
                    }
                    
                    if (_state.scanner.to < MAX_1G2_FREQ)
                    {
                        to = _state.scanner.to;
                    }

                    auto count = (to - from) / _scanerStep;
                    double rt = ((double)RSSI_BUFFER_SIZE / 2) / count;
                    uint8_t x = ((_scaner1G2->GetFreq() - from) / _scanerStep) * rt;
                    _state.rssi[x] = _scaner1G2->GetRssiA();

                    _scaner1G2->IncrementFreq(_scanerStep);
                }

                if (_scaner5G8->MeasureRSSI())
                {
                    message += _scaner5G8->MakeMessage();
                    uint16_t from = MIN_5G8_FREQ;
                    uint16_t to = MAX_5G8_FREQ;
                    if (_state.scanner.from >= MIN_5G8_FREQ)
                    {
                        from = _state.scanner.from;
                    }
                    
                    if (_state.scanner.to < MAX_5G8_FREQ)
                    {
                        to = _state.scanner.to;
                    }

                    auto count = (to - from) / _scanerStep;
                    double rt = ((double)RSSI_BUFFER_SIZE / 2) / count;
                    uint8_t x = ((_scaner5G8->GetFreq() - from) / _scanerStep) * rt;
                    _state.rssi[RSSI_BUFFER_SIZE / 2 + x] = _scaner5G8->GetRssiA();

                    _scaner5G8->IncrementFreq(_scanerStep);
                    scannerAuto = SET_FREQ_1G2;
                }
                
            break;
        }
    }

    return message;
}

void Terrestrial::SaveConfig()
{
    auto cfg = _config->GetTerrestrialConfig();
    cfg->receiverFreq = _state.receiver.currentFreq;
    cfg->from = _state.scanner.from;
    cfg->to = _state.scanner.to;
    cfg->filter = _state.scanner.filter;
    cfg->step = _state.scanner.step;
    _config->CommitTerrestrial();
}

void 
Terrestrial::Loop(uint32_t now)
{
    auto usStart = micros();
    ModuleBase::Loop(now);

    _remoteConsole->Loop();
    auto command = _remoteConsole->GetCommand();
    if (!command.empty())
    {
        auto mode = ParseCommand(command);
        SetWorkMode(mode);
        SaveConfig();
    }

    if (currentTimeMs == now)
    {
        return;
    }

    currentTimeMs = now;

    auto usI2CStart = micros();
    _userConsole->Loop();
    command = _userConsole->GetCommand();
    if (!command.empty())
    {
        auto mode = ParseCommand(command);
        SetWorkMode(mode);
        SaveConfig();
    }
    auto usStop = micros();
    uint8_t i2c = (usStop - usI2CStart) / 10;
    _state.device.i2c = i2c > 100 ? 100 : i2c;

    auto answer = Work();
    if (!answer.empty())
    {
        _remoteConsole->SendMessage(answer);
        _userConsole->SendMessage(answer);
    }

    usStop = micros();
    uint8_t cpu = (usStop - usStart) / 10;
    _state.device.cpu = cpu > 100 ? 100 : cpu;

    _state.device.connectionState = connectionState;
}

WORK_MODE_TYPE
Terrestrial::ParseCommand(const std::string& command)
{
    // Rxxxx\n -- set receiving on xxxx freq
    // Sxxxx:yyyy:dddd:ii\n -- set scanner mode from [xxxx] to [yyyy] with step [ii] MHz freq with delay [dddd] ms on each freq
    if (!command.empty())
    {
        if (command[0] == 'R')
        {
            if (command.size() >= 4)
            {
                _state.receiver.currentFreq = atoi(command.c_str() + 1);
                return RECEIVER;
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
                _scanerFilter = atoi(tmp);
                strncpy(tmp, buffer + 16, 2);
                tmp[2] = 0;
                _scanerStep  = atoi(tmp);

                if (minFreq > maxFreq)
                {
                    std::swap(minFreq, maxFreq);
                }

                minScaner1G2Freq = 0;
                maxScaner1G2Freq = 0;
                minScaner5G8Freq = 0;
                maxScaner5G8Freq = 0;
                if (minFreq <= MAX_1G2_FREQ)
                {
                    minScaner1G2Freq = minFreq;
                    maxScaner1G2Freq = MAX_1G2_FREQ;

                    if (maxFreq > MIN_5G8_FREQ)
                    {
                        minScaner5G8Freq = MIN_5G8_FREQ;
                        maxScaner5G8Freq = MAX_5G8_FREQ;
                    }
                }

                if (maxFreq <= MAX_1G2_FREQ)
                {
                    maxScaner1G2Freq = maxFreq;
                }

                if (minFreq >= MIN_5G8_FREQ && minFreq <= MAX_5G8_FREQ)
                {
                    minScaner5G8Freq = minFreq;
                    maxScaner5G8Freq = MAX_5G8_FREQ;
                }

                if (maxFreq >= MIN_5G8_FREQ && maxFreq <= MAX_5G8_FREQ)
                {
                    maxScaner5G8Freq = maxFreq;
                }

                _state.scanner.from = minFreq;
                _state.scanner.to = maxFreq;
                _state.scanner.step = _scanerStep;
                _state.scanner.filter = _scanerFilter;

                return SCANNER;
            }
        }
    }
    
    return NONE;
}

bool 
Terrestrial::CheckRSSI(ANTENNA_TYPE& antenna, uint16_t filterInitCounter)
{
    static uint16_t filter = 0;
    static uint32_t rssiASum = 0;
    static uint32_t rssiBSum = 0;

    if (filter == 0) filter = filterInitCounter;

    analogRead(RSSI_5G8_A);
    rssiASum += analogRead(RSSI_5G8_A);
    
    analogRead(RSSI_5G8_B);
    rssiBSum += analogRead(RSSI_5G8_B);
    if (--filter == 0)
    {
        _state.rssiA = rssiASum / filterInitCounter;
        _state.rssiB = rssiBSum / filterInitCounter;

        if (_state.rssiA - _state.rssiB > RSSI_DIFF_BORDER)
        {
            antenna = ANT_A;
        }
        else if (_state.rssiB - _state.rssiA > RSSI_DIFF_BORDER)
        {
            antenna = ANT_B;
        }

        rssiASum = 0;
        rssiBSum = 0;

        return true;
    }
 
    return false;
}

void
Terrestrial::SwitchVideo(ANTENNA_TYPE antenna)
{
    digitalWrite(VIDEO_CTRL, (antenna == ANT_A ? LOW : HIGH));
}
