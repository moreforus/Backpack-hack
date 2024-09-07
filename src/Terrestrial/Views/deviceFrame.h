#pragma once
#include <Arduino.h>
#include <Terrestrial/Views/baseFrame.h>

class OLEDDisplay;
class OLEDDisplayUiState;
class DeviceFrame : public BaseFrame
{
public:
    DeviceFrame(DEVICE_STATE* dataModel);
    virtual void SetActive(bool isActive) override
    {
        _state = WIDGET_STATE_TYPE::Preview;
    }

    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override {};

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override {};
    DEVICE_STATE* _dataModel;
};
