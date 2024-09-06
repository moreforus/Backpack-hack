#include <Terrestrial/userConsole.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include <iencoder.h>
#include <Terrestrial/menuItem.h>

std::string UserConsole::_commands[] = {"R", "S", "T"};

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
    _frames.push_back(new MenuItem([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 0 + y, "Receiver");
            display->setFont(ArialMT_Plain_10);
            display->drawString(0 + x, 18 + y, "Freq:");

            auto tmp = std::to_string(_state->receiver.freq);
            display->drawString(30 + x, 18 + y, tmp.c_str());
        }
        ));
    _frames.push_back(new MenuItem([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 0 + y, "Scanner");
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
    _frames.push_back(new MenuItem([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
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
        ));       
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
        if (state == IENCODER_STATE::PLUS)
        {
            _ui->nextFrame();
        }
        else if (state == IENCODER_STATE::MINUS)
        {
            _ui->previousFrame();
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
