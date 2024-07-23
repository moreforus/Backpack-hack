#include "rtc6712.h"
#include "logging.h"

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
    uint16_t r = RTC6712_FREQ_REF_MHZ;
    uint8_t a;
    uint16_t n;
    
    DBG("Setting index ");
    DBGLN("%x", index);

    float inputFreq = frequencyTable[index];
    float freq = inputFreq + RTC6712_INTERMEDIATE_FREQ;
    float remainder = freq - int(freq);
    if (remainder > 0)
    {
        if (remainder > 0.5)
        {
            remainder = 1.0 - remainder;
        }

        r = std::round(r / remainder);
    }

    freq = freq * r / RTC6712_FREQ_REF_MHZ;
    n = int(freq / 32);
    a = int(freq - n * 32);

    uint32_t regC;
    if (inputFreq < 1160)
    {
        regC = 0x10002c;
    }
    else if (inputFreq < 1280)
    {
        regC = 0x08002c;
    }
    else if (freq < 1360)
    {
        regC = 0x04002c;
    }
    else
    {
        regC = 0x00002c;
    }

    rtc6712WriteRegisters(r, a, n, regC);
}

void 
RTC6712::rtc6712WriteRegisters(uint16_t r, uint8_t a, uint16_t n, uint32_t regC)
{
    if (!SPIModeEnabled) 
    {
        EnableSPIMode();
    }

    uint32_t data;
    data = (a & 0x7f) | ((r & 0x7fff) << 7);
    rtc6712WriteRegister(RegAddrType::RegA, data);

    data = ((n & 0x1fff) << 9) | (RTC6712_CP_RF << 6) | (RTC6712_SC_CTRL << 5) | (0x3 << 3);
    rtc6712WriteRegister(RegAddrType::RegB, data);

    rtc6712WriteRegister(RegAddrType::RegC, regC);
}

void 
RTC6712::rtc6712WriteRegister(RegAddrType addr, uint32_t data)
{
    rtc6712WriteRegister((data << 2) | addr);
}

void 
RTC6712::rtc6712WriteRegister(uint32_t data)
{    
    uint32_t periodMicroSec = 1000000 / RTC6712_BIT_BANG_FREQ;
    
    digitalWrite(PIN_CS, LOW);
    delayMicroseconds(periodMicroSec);

    for (uint8_t i = 0; i < 24; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_MOSI, (data & 0x800000) ? HIGH : LOW );
        delayMicroseconds(periodMicroSec / 4);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec / 2);

        data <<= 1;
    }

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_CS, HIGH);

    delayMicroseconds(periodMicroSec * 10);
}
