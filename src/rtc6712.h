#pragma once

#include "module_base.h"
#include <Arduino.h>

#define RTC6712_BIT_BANG_FREQ (10000)
#define RTC6712_FREQ_REF_MHZ (8)
#define RTC6712_INTERMEDIATE_FREQ (480)
#define RTC6712_CP_RF (0)
#define RTC6712_SC_CTRL (1)

const float frequencyTable[48] = {
    1080, 1120, 1160, 1200, 1240, 1258, 1280, 1320,
    1360, 1105, 1110, 1120.5, 1130, 1145, 1150, 1160.5,
    1100, 1105, 1110, 1120.5, 1130, 1145, 1150, 1160.5,
    1100, 1105, 1110, 1120.5, 1130, 1145, 1150, 1160.5,
    1100, 1105, 1110, 1120.5, 1130, 1145, 1150, 1160.5,
    1300, 1305, 1310, 1320.5, 1330, 1345, 1350, 1360
};

enum RegAddrType {
    RegA = 0,
    RegB,
    RegC,
    RegD
};

class RTC6712 : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);

private:
    void EnableSPIMode();
    void rtc6712WriteRegisters(uint16_t r, uint8_t a, uint16_t n, uint32_t regC);
    void rtc6712WriteRegister(RegAddrType addr, uint32_t data);
    void rtc6712WriteRegister(uint32_t data);
    bool SPIModeEnabled = false;
};
