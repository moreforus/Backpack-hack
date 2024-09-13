#include <Terrestrial/userConsole.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include <iencoder.h>
#include <Terrestrial/Views/receiverFrame.h>
#include <Terrestrial/Views/scannerFrame.h>
#include <Terrestrial/Views/deviceFrame.h>

UserConsole::UserConsole(TERRESTRIAL_STATE* state)
    : _state(state)
{
    _display = new SSD1306Wire(0x3c, SDA, SCL);
    _ui = new OLEDDisplayUi(_display);
    _iEnc = new IncrementalEncoder();
}

void
UserConsole::Init()
{
    _ui->setTargetFPS(15);
    //_ui->setActiveSymbol(activeSymbol);
    //_ui->setInactiveSymbol(inactiveSymbol);
    _ui->setIndicatorPosition(BOTTOM);
    _ui->setIndicatorDirection(LEFT_RIGHT);
    _ui->setFrameAnimation(SLIDE_LEFT);

    _receiverFrame = new ReceiverFrame(&_state->receiver);
    _receiverFrame->OnExit([&](BaseFrame* sender, bool isSave)
    { 
        sender->SetActive(false);
        _ui->enableAllIndicators();
        if (isSave)
        {
            std::lock_guard<std::mutex> lock(_commandMutex);
            _command = "R" + std::to_string(_state->receiver.currentFreq);
        }
    });
    _frames.push_back((BaseFrame*)_receiverFrame);

    _scannerFrame = new ScannerFrame(&_state->scanner);
    _scannerFrame->OnExit([&](BaseFrame* sender, bool isSave)
    { 
        sender->SetActive(false);
        _ui->enableAllIndicators();
        if (isSave)
        {
            char tmp[64];
            sprintf(tmp, "S%04d:%04d:%04d:%02d", _state->scanner.from, _state->scanner.to, _state->scanner.filter, _state->scanner.step);
            std::lock_guard<std::mutex> lock(_commandMutex);
            _command = tmp;
        }
    });
    _frames.push_back(_scannerFrame);

    BaseFrame* deviceFrame = new DeviceFrame(&_state->device);
    _frames.push_back(deviceFrame);

    _ui->setFrames(_frames);

    // TODO: overlays: Battery, Loading CPU, Band:Channel, Freq
    //_ui->setOverlays(overlays, overlaysCount);

    _ui->init();
    _display->flipScreenVertically();
}

void
UserConsole::Loop()
{
    _iEnc->Poll();
    auto state = _iEnc->GetState();
    if (state != IENCODER_STATE::NONE)
    {
        auto baseFrame = (BaseFrame*)_frames[_ui->getCurrentFrame()];
        if ((baseFrame)->IsActive())
        {
            WIDGET_COMMAND_TYPE command = WIDGET_COMMAND_TYPE::NONE;
            if (state == IENCODER_STATE::PLUS)
            {
                command = WIDGET_COMMAND_TYPE::INCREMENT;
            }
            else if (state == IENCODER_STATE::MINUS)
            {
                command = WIDGET_COMMAND_TYPE::DECREMENT;
            }
            else if (state == IENCODER_STATE::BUTTON_SHORT_PRESS)
            {
                command = WIDGET_COMMAND_TYPE::ENTER;
            }
            else if (state == IENCODER_STATE::BUTTON_LONG_PRESS)
            {
                command = WIDGET_COMMAND_TYPE::CANCEL;
            }
            else if (state == IENCODER_STATE::BUTTON_START_PRESS)
            {
                command = WIDGET_COMMAND_TYPE::BUTTON_PRESS_STARTED;
            }

            baseFrame->SetCommand(command);
        }
        else
        {
            if (state == IENCODER_STATE::PLUS)
            {
                _ui->nextFrame();
            }
            else if (state == IENCODER_STATE::MINUS)
            {
                _ui->previousFrame();
            }
            else if (state == IENCODER_STATE::BUTTON_SHORT_PRESS)
            {
                baseFrame->SetActive(true);
                if (baseFrame->IsActive())
                {
                    _ui->disableAllIndicators();
                }
            }
            else if (state == IENCODER_STATE::BUTTON_LONG_PRESS)
            {
                baseFrame->SetActive(false);
                _ui->enableAllIndicators();
            }
        }
    }

    _ui->update();
}

std::string
UserConsole::GetCommand()
{
    std::lock_guard<std::mutex> lock(_commandMutex);
    auto tmp = _command;
    _command = "";
    return tmp;
}

void
UserConsole::SendMessage(const std::string& message)
{
    if (message[0] == 'R')
    {
        _receiverFrame->UpdateRSSI(_state->receiverState);
    }
    else if (message[0] == 'S')
    {
        _scannerFrame->UpdateRSSI(&_state->scannerState);
    }
}
