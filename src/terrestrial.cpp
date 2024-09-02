#include "terrestrial.h"
#include <SPI.h>
#include "logging.h"
#include <rtc6715.h>

void
Terrestrial::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);

    pinMode(VIDEO_CTRL, OUTPUT);

    DBGLN("Terrestrial init complete");
    Serial.begin(460800);
    Serial.setTimeout(1);
}

void
Terrestrial::EnableSPIMode()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    SPIModeEnabled = true;

    DBGLN("SPI config complete");
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

    rtc6715SetFreq(currentFreq);
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
                    minScannerFreq = atoi(tmp);
                    strncpy(tmp, buffer + 6, 4);
                    tmp[4] = 0;
                    maxScannerFreq = atoi(tmp);
                    strncpy(tmp, buffer + 11, 4);
                    tmp[4] = 0;
                    scanerFilter = atoi(tmp);
                    strncpy(tmp, buffer + 16, 2);
                    tmp[2] = 0;
                    scanerStep  = atoi(tmp);
                    pointer = 0;

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

        rtc6715SetFreq(currentFreq);
    }
}

std::string
Terrestrial::MakeMessage(const char* cmd)
{
    uint64_t us = micros();
    char str[48];
    sprintf(str, "%s:%d[%d:%d]%d>%llu", cmd, currentFreq, rssiA, rssiB, currentAntenna, us);
    
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

            auto message = MakeMessage("R");
            messageQueue.push_back(message);
        }
    }
    else if (workMode == SCANNER)
    {
        switch (scannerAuto)
        {
            case INIT:
                currentFreq = minScannerFreq;
                scannerAuto = SET_FREQ;
            break;
            case SET_FREQ:
                if (!SPIModeEnabled) 
                {
                    EnableSPIMode();
                }

                rtc6715SetFreq(currentFreq);
                scannerAuto = MEASURE;
            break;

            case MEASURE:
                ANTENNA_TYPE antenna = ANT_A;
                if (CheckRSSI(now, antenna, scanerFilter))
                {
                    auto message = MakeMessage("S");
                    messageQueue.push_back(message);

                    currentFreq += scanerStep;
                    if (currentFreq > maxScannerFreq)
                    {
                        currentFreq = minScannerFreq;
                    }

                    scannerAuto = SET_FREQ;
                }
            break;
        }
    }
}

void
Terrestrial::SendMessage()
{
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

    auto mode = ParseSerialCommand();
    SetWorkMode(mode);
    
    Work(now);

    SendMessage();
}


#define RSSI_DIFF_BORDER (16)

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

    analogRead(RSSI_A);
    rssiASum += analogRead(RSSI_A);
    
    analogRead(RSSI_B);
    rssiBSum += analogRead(RSSI_B);
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
