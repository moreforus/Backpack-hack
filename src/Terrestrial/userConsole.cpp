#include <Terrestrial/userConsole.h>
#include <SSD1306Wire.h>
#include <OLEDDisplayUi.h>
#include <iencoder.h>
#include <Terrestrial/Views/receiverFrame.h>
#include <Terrestrial/Views/scannerFrame.h>
#include <Terrestrial/Views/deviceFrame.h>
#include <Terrestrial/receiversParam.h>

UserConsole::UserConsole(TERRESTRIAL_STATE* state)
    : _state(state)
{
    _display = new SSD1306Wire(0x3c, SDA, SCL);
    _ui = new OLEDDisplayUi(_display);
    _iEnc = new IncrementalEncoder();
    _displayWidth = _display->width();
    _rssi = new uint16_t[_displayWidth];
    memset(_rssi, 0, _displayWidth * sizeof(uint16_t));
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
            _command.work = WORK_MODE_TYPE::RECEIVER;
            _command.freq = _state->receiver.currentFreq;
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
            std::lock_guard<std::mutex> lock(_commandMutex);
            _command.work = WORK_MODE_TYPE::SCANNER;
            _command.scannerFrom = _state->scanner.from;
            _command.scannerTo = _state->scanner.to;
            _command.scannerFilter = _state->scanner.filter;
            _command.scannerStep = _state->scanner.step;
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

    auto response = GetMessage();
    if (response.work == WORK_MODE_TYPE::RECEIVER)
    {
        _receiverFrame->UpdateRSSI(_state->receiverState);
        _response = response;
    }
    else if (response.work == WORK_MODE_TYPE::SCANNER)
    {
        if (_response.work != WORK_MODE_TYPE::SCANNER)
        {
            ScannerStart();
        }

        if (response.freq > MIN_5G8_FREQ)
        {
            PrepareBufferForDraw5G8(response);
        }
        else if (response.freq < MAX_1G2_FREQ)
        {
            PrepareBufferForDraw1G2(response);
        }

        _scannerFrame->UpdateRSSI(_rssi);
        _response = response;
    }

    _ui->update();
}

void
UserConsole::ScannerStart()
{
    _isScalingCompleted = false;
    _preX1g2 = 0;
    _preX5g8 = _displayWidth / 2;
    _rssi[_preX1g2] = 0;
    _rssi[_preX5g8] = 0;    
}

void
UserConsole::PrepareBufferForDraw1G2(const TerrestrialResponse_t& response)
{
    uint16_t from = MIN_1G2_FREQ;
    uint16_t to = MAX_1G2_FREQ;
    if (response.command.scannerFrom < MAX_1G2_FREQ)
    {
        from = response.command.scannerFrom;
    }
    
    if (response.command.scannerTo < MAX_1G2_FREQ)
    {
        to = response.command.scannerTo;
    }

    auto count = (to - from) / response.command.scannerStep;
    double rt = ((double)(_displayWidth - 1) / 2) / count;
    uint8_t x = (response.freq - from) * rt / response.command.scannerStep;
    if (x < _displayWidth)
    {
        if (x != _preX1g2)
        {
            _preX1g2 = x;
            _rssi[_preX1g2] = 0;
            if (x < _displayWidth / 2 - 1)
            {
                _rssi[_preX1g2 + 1] = 0;
            }
        }

        uint16_t rssi = response.GetMaxRssi() * _scale1G2;
        if (rssi > _rssi[x])
        {
            _rssi[x] = rssi;
        }
    }
}

void
UserConsole::PrepareBufferForDraw5G8(const TerrestrialResponse_t& response)
{
    uint16_t from = MIN_5G8_FREQ;
    uint16_t to = MAX_5G8_FREQ;
    if (response.command.scannerFrom >= MIN_5G8_FREQ)
    {
        from = response.command.scannerFrom;
    }
    
    if (response.command.scannerTo < MAX_5G8_FREQ)
    {
        to = response.command.scannerTo;
    }

    auto count = (to - from) / response.command.scannerStep;
    double rt = ((double)(_displayWidth - 1) / 2) / count;
    uint8_t x = _displayWidth / 2 + ((response.freq - from) * rt / response.command.scannerStep);
    if (x < _displayWidth)
    {
        if (x != _preX5g8)
        {
            _preX5g8 = x;
            _rssi[_preX5g8] = 0;
            if (x < _displayWidth - 1)
            {
                _rssi[_preX5g8 + 1] = 0;
            }
        }

        uint16_t rssi = response.GetMaxRssi() * _scale5G8;
        if (rssi > _rssi[x])
        {
            _rssi[x] = rssi;
        }
    }
}
