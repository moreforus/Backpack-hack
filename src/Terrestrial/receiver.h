#pragma once
#include <Arduino.h>
#include <common.h>
#include <rtc6715.h>
#include <lib_rtc6712.h>
#include <Terrestrial/receiversParam.h>
#include <Terrestrial/antennaType.h>

class Receiver
{
public:
    Receiver(uint8_t rssiA1G2pin, uint8_t rssiB1G2pin
            , uint8_t rssiA5G8pin, uint8_t rssiB5G8pin
            , uint8_t rssiDiff)
    : _rssiA1G2pin(rssiA1G2pin), 
      _rssiB1G2pin(rssiB1G2pin),
      _rssiA5G8pin(rssiA5G8pin), 
      _rssiB5G8pin(rssiB5G8pin),
      _rssiDiff(rssiDiff)
    {

    }

    uint16_t inline GetRssiA() const
    {
        return _rssiA;
    }

    uint16_t inline GetRssiB() const
    {
        return _rssiB;
    }

    void SetFreq(frequency_t freq)
    {
        if (freq >= MIN_5G8_FREQ)
        {
            rtc6715SetFreq(freq);
            _currentFreq = freq;
        }
        else if (freq < MAX_1G2_FREQ)
        {
            rtc6712SetFreq(freq);
            _currentFreq = freq;
        }
    }

    bool MeasureRSSI(ANTENNA_TYPE& antenna, uint16_t filterInitCounter)
    {
        static uint16_t filter = 0;
        static uint32_t rssiASum = 0;
        static uint32_t rssiBSum = 0;

        if (filter == 0) filter = filterInitCounter;

        if (_currentFreq >= MIN_5G8_FREQ)
        {
            rssiASum += analogRead(_rssiA5G8pin);
            rssiBSum += analogRead(_rssiB5G8pin);
        }
        else if (_currentFreq < MAX_1G2_FREQ)
        {
            rssiASum += analogRead(_rssiA1G2pin);
            rssiBSum += analogRead(_rssiB1G2pin);
        }

        if (--filter == 0)
        {
            _rssiA = rssiASum / filterInitCounter;
            _rssiB = rssiBSum / filterInitCounter;

            if (_rssiA - _rssiB > _rssiDiff)
            {
                antenna = ANT_A;
            }
            else if (_rssiB - _rssiA > _rssiDiff)
            {
                antenna = ANT_B;
            }

            rssiASum = 0;
            rssiBSum = 0;

            return true;
        }
    
        return false;
    }

private:
    const uint8_t _rssiA1G2pin;
    const uint8_t _rssiB1G2pin;
    const uint8_t _rssiA5G8pin;
    const uint8_t _rssiB5G8pin;
    const uint8_t _rssiDiff;

    frequency_t _currentFreq;
    uint16_t _rssiA;
    uint16_t _rssiB;
};
