#include <Terrestrial/userConsole.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include <iencoder.h>
#include <Terrestrial/Views/receiverFrame.h>

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
    BaseFrame* receiverFrame = new ReceiverFrame(&_state->receiver);
    receiverFrame->OnExit([&](BaseFrame* sender)
    { 
        sender->SetActive(false);
        _ui->enableAllIndicators();
    });
    _frames.push_back(receiverFrame);
    /*_frames.push_back(new BaseFrame([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 0 + y, "Scanner");

            //display->drawString(87 + x, 18 + y, "Start");
            //display->drawRect(85 + x, 18 + y, 39, 17);

            display->setFont(ArialMT_Plain_10);
            display->drawString(0 + x, 18 + y, "From:");
            display->drawString(0 + x, 28 + y, "To:");
            display->drawString(0 + x, 38 + y, "Step:");
            display->drawString(0 + x, 48 + y, "Filter:");

            auto tmp = std::to_string(_state->scanner.from);
            display->drawString(30 + x, 18 + y, tmp.c_str());
            tmp = std::to_string(_state->scanner.to);
            display->drawString(30 + x, 28 + y, tmp.c_str());
            tmp = std::to_string(_state->scanner.step);
            display->drawString(30 + x, 38 + y, tmp.c_str());
            tmp = std::to_string(_state->scanner.filter);
            display->drawString(30 + x, 48 + y, tmp.c_str());
        }
        ));
    _frames.push_back(new BaseFrame([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 0 + y, "Tasks");
            display->setFont(ArialMT_Plain_10);
            display->drawString(0 + x, 18 + y, "CPU:");
            display->drawString(0 + x, 28 + y, "I2C:");

            auto tmp = std::to_string(_state->device.cpu);
            display->drawString((tmp.size() == 1 ? 35 : 30) + x, 18 + y, tmp.c_str());
            tmp = std::to_string(_state->device.i2c);
            display->drawString((tmp.size() == 1 ? 34 : 30) + x, 28 + y, tmp.c_str());
        }
        ));*/
    _ui->setFrames(_frames);

    // TODO: overlays: Battery, Loading CPU, Band:Channel, Freq
    //_ui->setOverlays(overlays, overlaysCount);

    _ui->init();
    _display->flipScreenVertically();
}

void
UserConsole::Loop(uint32_t now)
{
    _iEnc->Poll(now);
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
                /*baseFrame->OnExit([&]()
                { 
                    baseFrame->SetActive(false);
                    _ui->enableAllIndicators();
                });*/

                _ui->disableAllIndicators();
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
    return "";
}

void
UserConsole::SendMessage(const std::string& message)
{
    auto nextFrame = _ui->getNextFrame();
    auto frame = _ui->getCurrentFrame();
    if (message[0] == 'R')
    {
        if ((nextFrame == -1 && frame != 0) || (nextFrame > -1 && nextFrame != 0))
        {
            //_ui->transitionToFrame(0);
        }
    }
    else if (message[0] == 'S')
    {
        if ((nextFrame == -1 && frame != 1) || (nextFrame > -1 && nextFrame != 1))
        {
            //_ui->transitionToFrame(1);
        }
    }
}
