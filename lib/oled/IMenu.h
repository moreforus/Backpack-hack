#pragma once
#include <Arduino.h>

class IMenu
{
public:
    virtual void Init() = 0;
    virtual void SetUserAction(uint8_t action) = 0;
    virtual void Loop() = 0;
};
