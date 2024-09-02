#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>
#include <string>
#include <vector>

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
    std::string MakeMessage(const char* cmd);

    bool SPIModeEnabled = false;
    ANTENNA_TYPE currentAntenna = ANT_A;
    uint32_t currentTimeMs = 0;
    uint16_t rssiA = 0;
    uint16_t rssiB = 0;
    uint16_t currentFreq = 0;
    WORK_MODE_TYPE workMode = RECEIVER;
    SCANNER_AUTO_TYPE scannerAuto;
    uint16_t minScannerFreq;
    uint16_t maxScannerFreq;
    uint16_t scanerFilter = 1;
    uint8_t scanerStep = 1;
    std::vector<std::string> messageQueue;
};
