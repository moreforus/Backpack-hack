#pragma once
#include <Arduino.h>
#include <common.h>
#include <rtc6715.h>
#include <lib_rtc6712.h>
#include <Terrestrial/receiversParam.h>
#include <Terrestrial/antennaType.h>

struct RECEIVER_PARAMS {
    uint8_t rssiA1G2pin;
    uint8_t rssiB1G2pin;
    uint8_t videoSelection1G2;
    uint8_t rssiA5G8pin;
    uint8_t rssiB5G8pin;
    uint8_t videoSelection5G8;
    uint8_t rssiDiff;
};

class Receiver
{
public:
    Receiver(const RECEIVER_PARAMS& receiverParams)
    : _rssiA1G2pin(receiverParams.rssiA1G2pin),
      _rssiB1G2pin(receiverParams.rssiB1G2pin),
      _videoSelection1G2(receiverParams.videoSelection1G2),
      _rssiA5G8pin(receiverParams.rssiA5G8pin),
      _rssiB5G8pin(receiverParams.rssiB5G8pin),
      _videoSelection5G8(receiverParams.videoSelection5G8),
      _rssiDiff(receiverParams.rssiDiff)
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

    void SwitchVideo(ANTENNA_TYPE antenna)
    {
        if (_currentFreq >= MIN_5G8_FREQ)
        {
            digitalWrite(_videoSelection5G8, (antenna == ANT_A ? LOW : HIGH));
        }
        else if (_currentFreq < MAX_1G2_FREQ)
        {
            digitalWrite(_videoSelection1G2, (antenna == ANT_A ? LOW : HIGH));
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
    const uint8_t _videoSelection1G2;
    const uint8_t _rssiA5G8pin;
    const uint8_t _rssiB5G8pin;
    const uint8_t _videoSelection5G8;
    const uint8_t _rssiDiff;

    frequency_t _currentFreq;
    uint16_t _rssiA;
    uint16_t _rssiB;
};
