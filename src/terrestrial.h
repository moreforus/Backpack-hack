#pragma once
#include "module_base.h"
#include <Arduino.h>
#include <channels.h>
#include <string>
#include <Terrestrial/scanner.h>
#include <Terrestrial/antennaType.h>
#include <Terrestrial/DataModel/terrestrialState.h>
#include <common.h>

enum WORK_MODE_TYPE : uint8_t
{
    NONE = 0,
    RECEIVER,
    SCANNER,
};

class RemoteConsole;
class UserConsole;
class Terrestrial : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);
    void UpdateConnectionState(connectionState_e connectionState);

private:
    void EnableSPIMode();
    bool CheckRSSI(uint32_t now, ANTENNA_TYPE& antenna, uint16_t filterInitCounter);
    void SwitchVideo(ANTENNA_TYPE antenna);
    WORK_MODE_TYPE ParseCommand(const std::string& command);
    void SetWorkMode(WORK_MODE_TYPE mode);
    std::string Work(uint32_t now);
    std::string MakeMessage(const char* cmd, const uint16_t freq);
    void SetFreq(uint16_t freq);
    void SaveConfig();

    bool SPIModeEnabled = false;
    ANTENNA_TYPE currentAntenna = ANT_A;
    uint32_t currentTimeMs = 0;
    WORK_MODE_TYPE workMode = RECEIVER;
    SCANNER_AUTO_TYPE scannerAuto;
    uint16_t minScaner5G8Freq;
    uint16_t maxScaner5G8Freq;
    uint16_t minScaner1G2Freq;
    uint16_t maxScaner1G2Freq;
    uint16_t _scanerFilter = 1;
    uint8_t _scanerStep = 1;
    Scaner* _scaner1G2 = nullptr;
    Scaner* _scaner5G8 = nullptr;
    RemoteConsole* _remoteConsole;
    UserConsole* _userConsole;
    TERRESTRIAL_STATE _state;
};
