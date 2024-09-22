#pragma once
#include <Terrestrial/IConsole.h>
#include <vector>
#include <Terrestrial/DataModel/terrestrialState.h>

class SSD1306Wire;
class OLEDDisplay;
class OLEDDisplayUi;
class OLEDDisplayUiState;
class IncrementalEncoder;
class IFrame;
class ReceiverFrame;
class ScannerFrame;
class UserConsole : public BaseConsole
{
public:
    UserConsole(TERRESTRIAL_STATE* state);
    virtual void Init() override;
    virtual void Loop() override;

private:
    void ScannerStart();
    void PrepareBufferForDraw1G2(const TerrestrialResponse_t& response);
    void PrepareBufferForDraw5G8(const TerrestrialResponse_t& response);

    uint8_t _commandIndex;
    SSD1306Wire* _display;
    OLEDDisplayUi* _ui;
    IncrementalEncoder* _iEnc;

    std::vector<IFrame*> _frames;
    TERRESTRIAL_STATE* _state;
    ReceiverFrame* _receiverFrame;
    ScannerFrame* _scannerFrame;

    double _scale1G2 = 1.0;
    double _scale5G8 = 1.0;
    uint8_t _preX1g2;
    uint8_t _preX5g8;
    bool _isScalingCompleted = false;
    uint16_t* _rssi;
    uint16_t _displayWidth;

    TerrestrialResponse_t _response;
};
