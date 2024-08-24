#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>

class RX5808 : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);

private:
    void EnableSPIMode();
    bool SPIModeEnabled = false;
};
