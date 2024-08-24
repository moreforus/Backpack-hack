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

    DBGLN("RX5808 init complete");
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
