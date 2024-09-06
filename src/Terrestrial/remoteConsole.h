#pragma once
#include <Arduino.h>
#include <Terrestrial/IConsole.h>
#include <mutex>
#include <string>
#include <vector>

class RemoteConsole : public IConsole
{
public:
    RemoteConsole(unsigned long baud);
    virtual void Init() override;
    virtual void Loop(uint32_t now) override;
    virtual std::string GetCommand() override;
    virtual void SendMessage(const std::string& message) override;

private:
    char _buffer[20] = { 0, 0, };
    uint8_t _pointer = 0;
    std::string _command;
    std::mutex _messageQueueMutex;
    std::vector<std::string> _messageQueue;
};
