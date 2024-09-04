#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>
#include <string>
#include <vector>
#include <mutex>

enum ANTENNA_TYPE : uint8_t
{
    ANT_A = 0,
    ANT_B,
};

enum WORK_MODE_TYPE : uint8_t
{
    NONE = 0,
    RECEIVER,
    SCANNER,
};

enum SCANNER_AUTO_TYPE : uint8_t
{
    INIT = 0,
    SET_FREQ,
    MEASURE,
};

#define RSSI_DIFF_BORDER (16)
typedef void (*worker)(uint16_t);

class Scaner {
public:
    Scaner(worker function, uint8_t rssiApin, uint8_t rssiBpin)
    : _rssiApin(rssiApin), 
      _rssiBpin(rssiBpin)
    {
        _worker = function;
    }

    void Init(uint16_t minFreq, uint16_t maxFreq)
    {
        _minFreq = minFreq;
        _maxFreq = maxFreq;
        _scanerFreq = _minFreq;
        _filter = 0;
        _rssiASum = 0;
        _rssiBSum = 0;
    }

    void SetFreq()
    {
        _worker(_scanerFreq);
    }

    bool MeasureRSSI(uint32_t now, uint16_t filterInitCounter)
    {
        if (now - _currentTimeMs < 1)
        {
            return false;
        }

        if (_filter == 0) _filter = filterInitCounter;

        _currentTimeMs = now;

        analogRead(_rssiApin);
        _rssiASum += analogRead(_rssiApin);
    
        analogRead(_rssiBpin);
        _rssiBSum += analogRead(_rssiBpin);
        if (--_filter == 0)
        {
            _rssiA = _rssiASum / filterInitCounter;
            _rssiB = _rssiBSum / filterInitCounter;

            if (_rssiA - _rssiB > RSSI_DIFF_BORDER)
            {
                _currentAntenna = ANT_A;
            }
            else if (_rssiB - _rssiA > RSSI_DIFF_BORDER)
            {
                _currentAntenna = ANT_B;
            }

            _rssiASum = 0;
            _rssiBSum = 0;

            return true;
        }
 
        return false;
    }

    uint16_t GetRssiA() const
    {
        return _rssiA;
    }

    uint16_t GetRssiB() const
    {
        return _rssiB;
    }

    std::string MakeMessage()
    {
        uint64_t us = micros();
        char str[48];
        sprintf(str, "S:%d[%d:%d]%d>%llu", _scanerFreq, _rssiA, _rssiB, _currentAntenna, us);
    
        return str;
    }

    void IncrementFreq(uint8_t step)
    {
        _scanerFreq += step;
        if (_scanerFreq > _maxFreq)
        {
            _scanerFreq = _minFreq;
        }
    }

private:
    worker _worker;
    uint16_t _scanerFreq;
    uint16_t _minFreq;
    uint16_t _maxFreq;
    uint16_t _rssiA = 0;
    uint16_t _rssiB = 0;
    uint16_t _filter = 0;
    uint32_t _rssiASum = 0;
    uint32_t _rssiBSum = 0;
    uint32_t _currentTimeMs = 0;
    const uint8_t _rssiApin;
    const uint8_t _rssiBpin;
    ANTENNA_TYPE _currentAntenna;
};

class Terrestrial : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);

private:
    void EnableSPIMode();
    bool CheckRSSI(uint32_t now, ANTENNA_TYPE& antenna, uint16_t filterInitCounter);
    void SwitchVideo(ANTENNA_TYPE antenna);
    WORK_MODE_TYPE ParseSerialCommand();
    void SetWorkMode(WORK_MODE_TYPE mode);
    void Work(uint32_t now);
    void SendMessage();
    std::string MakeMessage(const char* cmd, const uint16_t freq);
    void SetFreq(uint16_t freq);

    bool SPIModeEnabled = false;
    ANTENNA_TYPE currentAntenna = ANT_A;
    uint32_t currentTimeMs = 0;
    uint16_t rssiA = 0;
    uint16_t rssiB = 0;
    uint16_t currentFreq = 0;
    WORK_MODE_TYPE workMode = RECEIVER;
    SCANNER_AUTO_TYPE scannerAuto;
    uint16_t minScaner5G8Freq;
    uint16_t maxScaner5G8Freq;
    uint16_t minScaner1G2Freq;
    uint16_t maxScaner1G2Freq;
    uint16_t _scanerFilter = 1;
    uint8_t _scanerStep = 1;
    std::mutex _messageQueueMutex;
    std::vector<std::string> messageQueue;
    Scaner* _scaner1G2 = nullptr;
    Scaner* _scaner5G8 = nullptr;
};
