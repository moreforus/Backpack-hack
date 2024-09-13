#pragma once
#include "module_base.h"
#include <Arduino.h>
#include <channels.h>
#include <string>
#include <Terrestrial/scanner.h>
#include <Terrestrial/antennaType.h>
#include <Terrestrial/DataModel/terrestrialState.h>

enum WORK_MODE_TYPE : uint8_t
{
    NONE = 0,
    RECEIVER,
    SCANNER,
};

class RemoteConsole;
class UserConsole;
class VrxBackpackConfig;
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
    bool CheckRSSI(ANTENNA_TYPE& antenna, uint16_t filterInitCounter);
    void SwitchVideo(ANTENNA_TYPE antenna);
    WORK_MODE_TYPE ParseCommand(const std::string& command);
    void SetWorkMode(WORK_MODE_TYPE mode);
    std::string Work();
    std::string MakeMessage(const char* cmd, const uint16_t freq);
    void SetFreq(uint16_t freq);
    void SaveConfig();

    bool SPIModeEnabled = false;
    ANTENNA_TYPE currentAntenna = ANT_A;
    uint32_t currentTimeMs = 0;
    WORK_MODE_TYPE workMode = RECEIVER;
    SCANNER_AUTO_TYPE scannerAuto = SCANNER_AUTO_TYPE::INIT;
    uint16_t minScanner5G8Freq;
    uint16_t maxScanner5G8Freq;
    uint16_t minScanner1G2Freq;
    uint16_t maxScanner1G2Freq;
    uint16_t _scannerFilter = 1;
    uint8_t _scannerStep = 1;
    Scanner* _scanner1G2 = nullptr;
    Scanner* _scanner5G8 = nullptr;
    RemoteConsole* _remoteConsole;
    UserConsole* _userConsole;
    TERRESTRIAL_STATE _state;
    VrxBackpackConfig* _config;
    bool _isScalingCompleted = false;
    double _scale1G2 = 1.0;
    double _scale5G8 = 1.0;
};
