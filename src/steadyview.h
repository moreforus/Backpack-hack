#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>

typedef enum {
    ModeMix = 0,
    ModeDiversity
} videoMode_t;

class SteadyView : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SetMode(videoMode_t mode);

private:
    uint8_t currentIndex;
};
