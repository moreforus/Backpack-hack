#pragma once
#include <Arduino.h>
#include <string>

class IConsole
{
public:
    virtual void Init() = 0;
    virtual void Loop(uint32_t now) = 0;
    virtual std::string GetCommand() = 0;
    virtual void SendMessage(const std::string& message) = 0;
};