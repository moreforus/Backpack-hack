#include <lib_rtc6712.h>
#include <Arduino.h>

#define RTC6712_BIT_BANG_FREQ (10000)
#define RTC6712_FREQ_REF_MHZ (8)
#define RTC6712_INTERMEDIATE_FREQ (480)
#define RTC6712_CP_RF (0)
#define RTC6712_SC_CTRL (1)

enum RegAddrType : uint8_t {
    RegA = 0,
    RegB,
    RegC,
    RegD
};

void rtc6712WriteRegister(uint32_t data)
{    
    uint32_t periodMicroSec = 1;
    
    digitalWrite(PIN_1G2_CS, LOW);
    delayMicroseconds(periodMicroSec);

    for (uint8_t i = 0; i < 24; ++i)
    {
        digitalWrite(PIN_CLK, LOW);
        delayMicroseconds(periodMicroSec);
        digitalWrite(PIN_MOSI, (data & 0x800000) ? HIGH : LOW );
        delayMicroseconds(periodMicroSec);
        digitalWrite(PIN_CLK, HIGH);
        delayMicroseconds(periodMicroSec);

        data <<= 1;
    }

    digitalWrite(PIN_CLK, LOW);
    delayMicroseconds(periodMicroSec);
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_1G2_CS, HIGH);

    delayMicroseconds(periodMicroSec * 10);
}

void rtc6712WriteRegister(RegAddrType addr, uint32_t data)
{
    rtc6712WriteRegister((data << 2) | addr);
}

void rtc6712WriteRegisters(uint16_t r, uint8_t a, uint16_t n, uint32_t regC)
{
    uint32_t data;
    data = (a & 0x7f) | ((r & 0x7fff) << 7);
    rtc6712WriteRegister(RegAddrType::RegA, data);

    data = ((n & 0x1fff) << 9) | (RTC6712_CP_RF << 6) | (RTC6712_SC_CTRL << 5) | (0x3 << 3);
    rtc6712WriteRegister(RegAddrType::RegB, data);

    rtc6712WriteRegister(RegAddrType::RegC, regC);
}

void rtc6712SetFreq(frequency_t inputFreq)
{
    uint16_t r = RTC6712_FREQ_REF_MHZ;
    uint8_t a;
    uint16_t n;

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
