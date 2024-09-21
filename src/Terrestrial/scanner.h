#pragma once
#include <Terrestrial/antennaType.h>
#include <common.h>

enum SCANNER_AUTO_TYPE : uint8_t
{
    INIT = 0,
    SET_FREQ_1G2,
    SET_FREQ_5G8,
    MEASURE,
};

struct ScannerRssiState_t
{
    uint16_t rssiMin;
    uint16_t rssiMax;
    frequency_t freqForMaxValue;
};

typedef void (*worker)(frequency_t);

class Scanner {
public:
    Scanner(worker function, uint8_t rssiApin, uint8_t rssiBpin, uint8_t rssiDiff)
    : _rssiApin(rssiApin), 
      _rssiBpin(rssiBpin),
      _rssiDiff(rssiDiff)
    {
        _worker = function;
    }

    void Init(frequency_t minFreq, frequency_t maxFreq)
    {
        _minFreq = minFreq;
        _maxFreq = maxFreq;
        _scannerFreq = _minFreq;
        _filter = 0;
        _rssiASum = 0;
        _rssiBSum = 0;
        ResetScannerRssiState();
    }

    void SetFreq(uint16_t filterInitCounter)
    {
        _worker(_scannerFreq);
        _filterInitCounter = filterInitCounter;
        _filter = filterInitCounter;
    }

    frequency_t inline GetFreq() const
    {
        return _scannerFreq;
    }

    bool MeasureRSSI()
    {
        if (_filter == 0)
        {
            return false;
        }

        _rssiASum += analogRead(_rssiApin);
        _rssiBSum += analogRead(_rssiBpin);
        if (--_filter == 0)
        {
            _rssiA = _rssiASum / _filterInitCounter;
            _rssiB = _rssiBSum / _filterInitCounter;

            if (_rssiA >= _rssiB)
            {
                _currentAntenna = ANT_A;
                if (_scannerRssiState.rssiMax < _rssiA)
                {
                    _scannerRssiState.rssiMax = _rssiA;
                    _scannerRssiState.freqForMaxValue = _scannerFreq;
                }

                /*if (_rssiB > 50 && _rssiB < _scannerRssiState.rssiMin)
                {
                    _scannerRssiState.rssiMin = _rssiB;
                }*/
                if (_rssiA < _scannerRssiState.rssiMin)
                {
                    _scannerRssiState.rssiMin = _rssiA;
                }
            }
            else
            {
                _currentAntenna = ANT_B;
                if (_scannerRssiState.rssiMax < _rssiB)
                {
                    _scannerRssiState.rssiMax = _rssiB;
                    _scannerRssiState.freqForMaxValue = _scannerFreq;
                }

                if (_rssiA < _scannerRssiState.rssiMin)
                {
                    _scannerRssiState.rssiMin = _rssiA;
                }
            }

            _rssiASum = 0;
            _rssiBSum = 0;

            return true;
        }
 
        return false;
    }

    uint16_t inline GetMaxRssi() const
    {
        return _rssiA > _rssiB ? _rssiA : _rssiB;
    }

    uint16_t inline GetRssiA() const
    {
        return _rssiA;
    }

    uint16_t inline GetRssiB() const
    {
        return _rssiB;
    }

    bool IncrementFreq(frequency_t step)
    {
        _scannerFreq += step;
        if (_scannerFreq > _maxFreq)
        {
            _scannerFreq = _minFreq;
            return true;
        }

        return false;
    }

    ScannerRssiState_t inline GetRssiState() const
    {
        return _scannerRssiState;
    }

    void ResetScannerRssiState()
    {
        _scannerRssiState.rssiMin = 4096;
        _scannerRssiState.rssiMax = 0;
        _scannerRssiState.freqForMaxValue = 0;
    }

private:
    worker _worker;
    frequency_t _scannerFreq;
    frequency_t _minFreq;
    frequency_t _maxFreq;
    uint16_t _rssiA = 0;
    uint16_t _rssiB = 0;
    uint16_t _filter = 0;
    uint32_t _rssiASum = 0;
    uint32_t _rssiBSum = 0;
    const uint8_t _rssiApin;
    const uint8_t _rssiBpin;
    const uint8_t _rssiDiff;
    ANTENNA_TYPE _currentAntenna;
    uint16_t _filterInitCounter;
    ScannerRssiState_t _scannerRssiState;    
};
