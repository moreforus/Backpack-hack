#pragma once
#include <Arduino.h>
#include <Terrestrial/IConsole.h>
#include <mutex>
#include <string>
#include <vector>
#include <Terrestrial/terrestrialState.h>

class SSD1306Wire;
class OLEDDisplay;
class OLEDDisplayUi;
class OLEDDisplayUiState;
class IncrementalEncoder;
class IFrame;
class UserConsole : public IConsole
{
public:
    UserConsole(TERRESTRIAL_STATE* state);
    virtual void Init() override;
    virtual void Loop(uint32_t now) override;
    virtual std::string GetCommand() override;
    virtual void SendMessage(const std::string& message) override;

private:
    static std::string _commands[];
    uint8_t _commandIndex;
    SSD1306Wire* _display;
    OLEDDisplayUi* _ui;
    IncrementalEncoder* _iEnc;

    std::string _command;
    std::mutex _messageQueueMutex;
    std::vector<std::string> _messageQueue;

    std::vector<IFrame*> _frames;
    const TERRESTRIAL_STATE* _state;
};
