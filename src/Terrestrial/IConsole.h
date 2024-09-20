#pragma once
#include <string>

class IConsole
{
public:
    virtual void Init() = 0;
    virtual void Loop() = 0;
    virtual std::string GetCommand() = 0;
    virtual void SendMessage(const std::string& message) = 0;
};
