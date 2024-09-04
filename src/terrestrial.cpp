#include "terrestrial.h"
#include <SPI.h>
#include "logging.h"
#include <rtc6715.h>
#include <lib_rtc6712.h>

void
Terrestrial::Init()
{
    ModuleBase::Init();
    
    // common MOSI and CLK pins
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_5G8_CS, INPUT);
    pinMode(PIN_1G2_CS, INPUT);
    pinMode(VIDEO_CTRL, OUTPUT);

    DBGLN("Terrestrial init complete");
    Serial.begin(460800);
    Serial.setTimeout(1);

    _scaner1G2 = new Scaner(rtc6712SetFreq, RSSI_1G2_A, RSSI_1G2_B);
    _scaner5G8 = new Scaner(rtc6715SetFreq, RSSI_5G8_A, RSSI_5G8_B);
    _iEnc = new IncrementalEncoder();
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

#define MIN_1G2_FREQ (500)
#define MAX_1G2_FREQ (2500)

#define MIN_5G8_FREQ (4900)
#define MAX_5G8_FREQ (6000)

void
Terrestrial::SetFreq(uint16_t freq)
{
    if (freq >= MIN_5G8_FREQ)
    {
        rtc6715SetFreq(freq);
    }
    else if (freq < MAX_1G2_FREQ)
    {
        rtc6712SetFreq(freq);
    }
}

void
Terrestrial::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    currentFreq = frequencyTable[index];
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    SetFreq(currentFreq);
}

WORK_MODE_TYPE
Terrestrial::ParseSerialCommand()
{
    static char buffer[20] = { 0, 0, };
    static uint8_t pointer = 0;

    // Rxxxx\n -- set receiving on xxxx freq
    // Sxxxx:yyyy:dddd:ii\n -- set scanner mode from [xxxx] to [yyyy] with step [ii] MHz freq with delay [dddd] ms on each freq
    auto data = Serial.read();
    if (data > 0)
    {
        if ((data == '\n' || data == '\r') && pointer)
        {
            if (buffer[0] == 'R')
            {
                if (pointer >= 4)
                {
                    currentFreq = atoi(buffer + 1);
                    pointer = 0;

                    return RECEIVER;
                }
            }
            else if (buffer[0] == 'S')
            {
                if (pointer >= 17)
                {
                    char tmp[5];
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
                    pointer = 0;


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

                    return SCANNER;
                }
            }
            else
            {
                pointer = 0;
            }
        }

        // check the first byte. It must be a command
        if (pointer == 0)
        {
            if (data != 'S' && data != 'R')
            {
                return NONE;
            }
        }

        buffer[pointer] = data;
        ++pointer;
    }
    
    return NONE;
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

        SetFreq(currentFreq);
    }
}

std::string
Terrestrial::MakeMessage(const char* cmd, const uint16_t freq)
{
    uint64_t us = micros();
    char str[48];
    sprintf(str, "%s:%d[%d:%d]%d>%llu", cmd, freq, rssiA, rssiB, currentAntenna, us);
    
    return str;
}

#define DEFAULT_RECEIVER_FILTER (8)

void
Terrestrial::Work(uint32_t now)
{
    if (workMode == RECEIVER)
    {
        ANTENNA_TYPE antenna = ANT_A;
        if (CheckRSSI(now, antenna, DEFAULT_RECEIVER_FILTER))
        {
            if (antenna != currentAntenna)
            {
                currentAntenna = antenna;
                SwitchVideo(currentAntenna);
            }

            auto message = MakeMessage("R", currentFreq);
            std::lock_guard<std::mutex> _lock(_messageQueueMutex);
            messageQueue.push_back(message);
        }
    }
    else if (workMode == SCANNER)
    {
        switch (scannerAuto)
        {
            case INIT:
                _scaner1G2->Init(minScaner1G2Freq, maxScaner1G2Freq);
                _scaner5G8->Init(minScaner5G8Freq, maxScaner5G8Freq);

                scannerAuto = SET_FREQ;
            break;
            case SET_FREQ:
                if (!SPIModeEnabled) 
                {
                    EnableSPIMode();
                }

                _scaner1G2->SetFreq();
                _scaner5G8->SetFreq();

                scannerAuto = MEASURE;
            break;

            case MEASURE:
                if (_scaner1G2->MeasureRSSI(now, _scanerFilter))
                {
                    auto message = _scaner1G2->MakeMessage();
                    std::lock_guard<std::mutex> _lock(_messageQueueMutex);
                    messageQueue.push_back(message);
                    _scaner1G2->IncrementFreq(_scanerStep);

                    scannerAuto = SET_FREQ;
                }

                if (_scaner5G8->MeasureRSSI(now, _scanerFilter))
                {
                    auto message = _scaner5G8->MakeMessage();
                    std::lock_guard<std::mutex> _lock(_messageQueueMutex);
                    messageQueue.push_back(message);
                    _scaner5G8->IncrementFreq(_scanerStep);

                    scannerAuto = SET_FREQ;
                }
            break;
        }
    }
}

void
Terrestrial::SendMessage()
{
    std::lock_guard<std::mutex> _lock(_messageQueueMutex);
    if (messageQueue.size() > 0)
    {
        auto tmp = messageQueue.front();
        messageQueue.erase(messageQueue.begin());
        Serial.println(tmp.c_str());
    }
}

void 
Terrestrial::Loop(uint32_t now)
{
    ModuleBase::Loop(now);
    _iEnc->Poll(now);
    auto state = _iEnc->GetState();
    if (state != IENCODER_STATE::NONE)
    {
        //Serial.printf("%d", (uint8_t)state);
        //Serial.println();
    }

    auto mode = ParseSerialCommand();
    SetWorkMode(mode);
    Work(now);
    SendMessage();
}

bool 
Terrestrial::CheckRSSI(uint32_t now, ANTENNA_TYPE& antenna, uint16_t filterInitCounter)
{
    static uint16_t filter = 0;
    static uint32_t rssiASum = 0;
    static uint32_t rssiBSum = 0;

    if (now - currentTimeMs < 1)
    {
        return false;
    }

    if (filter == 0) filter = filterInitCounter;

    currentTimeMs = now;

    analogRead(RSSI_5G8_A);
    rssiASum += analogRead(RSSI_5G8_A);
    
    analogRead(RSSI_5G8_B);
    rssiBSum += analogRead(RSSI_5G8_B);
    if (--filter == 0)
    {
        rssiA = rssiASum / filterInitCounter;
        rssiB = rssiBSum / filterInitCounter;

        if (rssiA - rssiB > RSSI_DIFF_BORDER)
        {
            antenna = ANT_A;
        }
        else if (rssiB - rssiA > RSSI_DIFF_BORDER)
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
