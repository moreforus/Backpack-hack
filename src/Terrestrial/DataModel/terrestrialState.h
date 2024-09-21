#pragma once
#include <common.h>
#include <mutex>

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

#define RSSI_BUFFER_SIZE (128)

struct SCANNER_STATE
{
    uint16_t rssi[RSSI_BUFFER_SIZE];

    frequency_t GetMaxFreq1G2()
    {
        std::lock_guard<std::mutex> lock(_maxFreq1g2Mutex);
        return _maxFreq1g2;
    }

    frequency_t GetMaxFreq5G8()
    {
        std::lock_guard<std::mutex> lock(_maxFreq5g8Mutex);
        return _maxFreq5g8;
    }

    void SetMaxFreq1G2(frequency_t freq)
    {
        std::lock_guard<std::mutex> lock(_maxFreq1g2Mutex);
        _maxFreq1g2 = freq;
    }

    void SetMaxFreq5G8(frequency_t freq)
    {
        std::lock_guard<std::mutex> lock(_maxFreq5g8Mutex);
        _maxFreq5g8 = freq;
    }

private:
    std::mutex _maxFreq1g2Mutex;
    std::mutex _maxFreq5g8Mutex;
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
