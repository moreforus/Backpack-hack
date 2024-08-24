#include "rtc6712.h"
#include "logging.h"
#include <lib_rtc6712.h>

void
RTC6712::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_CS, INPUT);

    DBGLN("RTC6712 init complete");
}

void
RTC6712::EnableSPIMode()
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
RTC6712::SendIndexCmd(uint8_t index)
{
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    DBG("Setting index ");
    DBGLN("%x", index);

    float inputFreq = frequencyTable[index];
    rtc6712SetFreq(inputFreq);
}
