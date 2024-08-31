#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>

enum ANTENNA_TYPE : uint8_t
{
    ANT_A = 0,
    ANT_B,
};

class Terrestrial : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void Loop(uint32_t now);

private:
    void EnableSPIMode();
    ANTENNA_TYPE CheckRSSI(uint32_t now);
    void SwitchVideo(ANTENNA_TYPE antenna);

    bool SPIModeEnabled = false;
    ANTENNA_TYPE currentAntenna = ANT_A;
    uint32_t currentTimeMs = 0;
    uint16_t rssiA = 0;
    uint16_t rssiB = 0;
};
