#pragma once
#include "module_base.h"
#include <Arduino.h>
#include <Terrestrial/antennaType.h>
#include <Terrestrial/DataModel/terrestrialState.h>
#include <Terrestrial/workMode.h>

enum SCANNER_AUTO_TYPE : uint8_t
{
    INIT = 0,
    SET_FREQ_1G2,
    SET_FREQ_5G8,
    MEASURE,
};

class IConsole;
class VrxBackpackConfig;
class Scanner;
class Receiver;
class Terrestrial : public ModuleBase
{
public:
    Terrestrial(VrxBackpackConfig* config) : _config(config)
    {
    }

    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);

private:
    void EnableSPIMode();
    void SwitchVideo(ANTENNA_TYPE antenna);
    void SetWorkMode(WORK_MODE_TYPE mode);
    void Work();
    void SetFreq(frequency_t freq);
    void SaveConfig();
    TerrestrialResponse_t Receive();
    TerrestrialResponse_t ScannerMeasure(Scanner* _scanner);

    ANTENNA_TYPE _currentAntenna = ANT_A;
    uint32_t _currentTimeMs = 0;
    WORK_MODE_TYPE _workMode = RECEIVER;
    SCANNER_AUTO_TYPE _scannerAuto = SCANNER_AUTO_TYPE::INIT;
    frequency_t _minScanner5G8Freq;
    frequency_t _maxScanner5G8Freq;
    frequency_t _minScanner1G2Freq;
    frequency_t _maxScanner1G2Freq;
    uint16_t _scannerFilter = 1;
    frequency_t _scannerStep = 1;
    Scanner* _scanner1G2 = nullptr;
    Scanner* _scanner5G8 = nullptr;
    Receiver* _receiver;
    TERRESTRIAL_STATE _state;
    VrxBackpackConfig* _config;
    TerrestrialCommand_t _currentCommand;
    bool _isScalingCompleted = false;
};
