#include "rx5808.h"
#include <SPI.h>
#include "logging.h"
#include <rtc6715.h>

void
RX5808::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);

    #if defined(PIN_CS_2)
        pinMode(PIN_CS_2, INPUT);
    #endif

#if defined(VIDEO_CTRL)
    pinMode(VIDEO_CTRL, OUTPUT);
#endif

    DBGLN("RX5808 init complete");
    //Serial.begin(115200);
}

void
RX5808::EnableSPIMode()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);
    
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    #if defined(PIN_CS_2)
        pinMode(PIN_CS_2, OUTPUT);
        digitalWrite(PIN_CS_2, HIGH);
    #endif

    SPIModeEnabled = true;

    DBGLN("SPI config complete");
}

void
RX5808::SendIndexCmd(uint8_t index)
{
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    DBG("Setting index ");
    DBGLN("%x", index);

    uint16_t f = frequencyTable[index];
    rtc6715SetFreq(f);
}

void 
RX5808::Loop(uint32_t now)
{
    ModuleBase::Loop(now);
#ifdef RSSI_A
    ANTENNA_TYPE antenna = CheckRSSI(now);
    if (antenna != currentAntenna)
    {
        currentAntenna = antenna;
        SwitchVideo(currentAntenna);
    }
#endif
}

#define RSSI_FILTER (8)
#define RSSI_DIFF_BORDER (16)

ANTENNA_TYPE 
RX5808::CheckRSSI(uint32_t now)
{
    static uint8_t filter = RSSI_FILTER;
    ANTENNA_TYPE antenna = currentAntenna;

    if (now - currentTimeMs < 1)
    {
        return antenna;
    }

    currentTimeMs = now;

    analogRead(RSSI_A);
    rssiA += analogRead(RSSI_A);
    
    analogRead(RSSI_B);
    rssiB += analogRead(RSSI_B);
    if (--filter == 0)
    {
        rssiA /= RSSI_FILTER;
        rssiB /= RSSI_FILTER;

        if (rssiA - rssiB > RSSI_DIFF_BORDER)
        {
            antenna = ANT_A;
        }
        else if (rssiB - rssiA > RSSI_DIFF_BORDER)
        {
            antenna = ANT_B;
        }

        //Serial.printf("%d %d %d", rssiA, rssiB, antenna);
        //Serial.println();

        rssiA = 0;
        rssiB = 0;
        filter = RSSI_FILTER;
    }
 
    return antenna;
}

void
RX5808::SwitchVideo(ANTENNA_TYPE antenna)
{
    digitalWrite(VIDEO_CTRL, (antenna == ANT_A ? LOW : HIGH));
}
