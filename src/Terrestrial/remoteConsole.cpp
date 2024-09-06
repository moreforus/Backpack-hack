#include <Terrestrial/remoteConsole.h>

RemoteConsole::RemoteConsole(unsigned long baud)
{
    Serial.begin(baud);
    Serial.setTimeout(1);
}

void
RemoteConsole::Init()
{

}

void
RemoteConsole::Loop(uint32_t now)
{
    if (Serial.availableForWrite())
    {
        std::lock_guard<std::mutex> _lock(_messageQueueMutex);
        if (_messageQueue.size() > 0)
        {
            auto message = _messageQueue.front();
            _messageQueue.erase(_messageQueue.begin());
            Serial.println(message.c_str());
        }
    }

    auto data = Serial.read();
    if (data > 0)
    {
        if ((data == '\n' || data == '\r'))
        {
            if (_pointer > 0)
            {
                _buffer[_pointer] = 0;
                _command = _buffer;
            }

            _pointer = 0;
            return;
        }

        _buffer[_pointer] = data;
        ++_pointer;
    }
}

std::string
RemoteConsole::GetCommand()
{
    auto tmp = _command;
    _command = "";
    return tmp;
}

void
RemoteConsole::SendMessage(const std::string& message)
{
    std::lock_guard<std::mutex> _lock(_messageQueueMutex);
    _messageQueue.push_back(message);
}
