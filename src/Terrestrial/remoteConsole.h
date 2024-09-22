#pragma once
#include <Terrestrial/IConsole.h>

class RemoteConsole : public BaseConsole
{
public:
    RemoteConsole(unsigned long baud);
    virtual void Init() override;
    virtual void Loop() override;

private:
    std::string MakeMessage(const TerrestrialResponse_t& response);
    TerrestrialCommand_t ParseCommand(const std::string& command);

    char _buffer[20] = { 0, 0, };
    uint8_t _pointer = 0;
};
