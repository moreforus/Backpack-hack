#pragma once
#include <common.h>

struct RECEIVER_SETTINGS
{
    frequency_t currentFreq;
};

struct SCANNER_SETTINGS
{
    frequency_t from = 1000;
    frequency_t to = 6000;
    uint16_t filter = 10;
    frequency_t step = 4;
};

struct DEVICE_STATE
{
    uint8_t cpu0;
    uint8_t cpu1;
    uint8_t i2c;
    uint8_t battery;
    connectionState_e connectionState;
};

struct RECEIVER_STATE
{
    uint16_t rssiA;
    uint16_t rssiB;
};

struct SCANNER_STATE
{
    frequency_t GetMaxFreq1G2()
    {
        return _maxFreq1g2;
    }

    frequency_t GetMaxFreq5G8()
    {
        return _maxFreq5g8;
    }

    void SetMaxFreq1G2(frequency_t freq)
    {
        _maxFreq1g2 = freq;
    }

    void SetMaxFreq5G8(frequency_t freq)
    {
        _maxFreq5g8 = freq;
    }

private:
    frequency_t _maxFreq1g2;
    frequency_t _maxFreq5g8;
};

struct TERRESTRIAL_STATE
{
    RECEIVER_SETTINGS receiver;
    SCANNER_SETTINGS scanner;
    DEVICE_STATE device;
    RECEIVER_STATE receiverState;
    SCANNER_STATE scannerState;
};
