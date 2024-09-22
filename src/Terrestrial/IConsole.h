#pragma once
#include <Terrestrial/workMode.h>
#include <mutex>
#include <vector>

class IConsole
{
public:
    virtual void Init() = 0;
    virtual void Loop() = 0;
    virtual TerrestrialCommand_t GetCommand() = 0;
    virtual void SendMessage(const TerrestrialResponse_t& message) = 0;
};

class BaseConsole : public IConsole
{
public:
    virtual TerrestrialCommand_t GetCommand() override
    {
        std::lock_guard<std::mutex> lock(_commandMutex);
        auto tmp = _command;
        _command.work = WORK_MODE_TYPE::NONE;
        return tmp;
    }

    virtual void SendMessage(const TerrestrialResponse_t& message) override
    {
        std::lock_guard<std::mutex> _lock(_messageQueueMutex);
        _messageQueue.push_back(message);
    }

protected:
    TerrestrialResponse_t GetMessage()
    {
        TerrestrialResponse_t message;
        message.work = WORK_MODE_TYPE::NONE;

        std::lock_guard<std::mutex> _lock(_messageQueueMutex);
        if (_messageQueue.size() > 0)
        {
            message = _messageQueue.front();
            _messageQueue.erase(_messageQueue.begin());
        }

        return message;
    }

    std::mutex _commandMutex;
    TerrestrialCommand_t _command;

private:
    std::mutex _messageQueueMutex;
    std::vector<TerrestrialResponse_t> _messageQueue;
};
