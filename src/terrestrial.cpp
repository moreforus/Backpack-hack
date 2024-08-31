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
    static char buffer[16] = { 0, 0, };
    static uint8_t pointer = 0;

    // Rxxxx\n -- set receiving on xxxx freq
    // Sxxxx:yyyy:dddd\n -- set scanner mode from xxxx to yyyy freq with delay dddd ms on each freq
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
                if (pointer >= 9)
                {
                    char tmp[5];
                    strncpy(tmp, buffer + 1, 4);
                    minScannerFreq = atoi(tmp);
                    strncpy(tmp, buffer + 6, 4);
                    maxScannerFreq = atoi(tmp);
                    strncpy(tmp, buffer + 11, 4);
                    scanerDelay = atoi(tmp);
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
Terrestrial::Loop(uint32_t now)
{
    static uint16_t delay = 0;
    static uint32_t preNow = 0;

    ModuleBase::Loop(now);

    auto mode = ParseSerialCommand();
    if (mode != NONE)
    {
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

    uint64_t us = micros();
    if (workMode == RECEIVER)
    {
        ANTENNA_TYPE antenna = ANT_A;
        if (CheckRSSI(now, antenna))
        {
            if (antenna != currentAntenna)
            {
                currentAntenna = antenna;
                SwitchVideo(currentAntenna);
            }

            Serial.printf("R:%d[%d:%d]%d>%llu", currentFreq, rssiA, rssiB, currentAntenna, us);
            Serial.println();
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
                delay = 0;
                preNow = now;
                scannerAuto = DELAY;
            break;
            case DELAY:
                if (now - preNow > 0)
                {
                    preNow = now;
                    ++delay;
                    if (delay == scanerDelay)
                    {
                        scannerAuto = MEASURE;
                    }
                }
            break;
            case MEASURE:
                ANTENNA_TYPE antenna = ANT_A;
                if (CheckRSSI(now, antenna))
                {
                    Serial.printf("S:%d[%d:%d]%d>%llu", currentFreq, rssiA, rssiB, currentAntenna, us);
                    Serial.println();
                    ++currentFreq;
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

#define RSSI_FILTER (8)
#define RSSI_DIFF_BORDER (16)

bool 
Terrestrial::CheckRSSI(uint32_t now, ANTENNA_TYPE& antenna)
{
    static uint8_t filter = RSSI_FILTER;
    static uint16_t rssiASum = 0;
    static uint16_t rssiBSum = 0;

    if (now - currentTimeMs < 1)
    {
        return false;
    }

    currentTimeMs = now;

    analogRead(RSSI_A);
    rssiASum += analogRead(RSSI_A);
    
    analogRead(RSSI_B);
    rssiBSum += analogRead(RSSI_B);
    if (--filter == 0)
    {
        rssiA = rssiASum / RSSI_FILTER;
        rssiB = rssiBSum / RSSI_FILTER;

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
        filter = RSSI_FILTER;

        return true;
    }
 
    return false;
}

void
Terrestrial::SwitchVideo(ANTENNA_TYPE antenna)
{
    digitalWrite(VIDEO_CTRL, (antenna == ANT_A ? LOW : HIGH));
}
