#include "steadyview.h"
#include <SPI.h>
#include "logging.h"
#include <rtc6715.h>

void
SteadyView::Init()
{
    ModuleBase::Init();
    
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_CS, OUTPUT);

    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_MOSI, HIGH);
    digitalWrite(PIN_CS, HIGH);

    DBGLN("SPI config complete");
    delay(100);
    SetMode(ModeMix);
}

void
SteadyView::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    uint16_t f = frequencyTable[index];
    rtc6715SetFreq(f);

    currentIndex = index;
}

void
SteadyView::SetMode(videoMode_t mode)
{
    if (mode == ModeMix)
    {
        digitalWrite(PIN_CLK, HIGH);
        delay(100);
        digitalWrite(PIN_CLK, LOW);
        delay(500);
    }
    uint16_t f = frequencyTable[currentIndex];
    rtc6715SetFreq(f);
    delayMicroseconds(500);
    rtc6715SetFreq(f);
}
