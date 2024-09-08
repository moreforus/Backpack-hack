#pragma once
#include <Arduino.h>
#include <Terrestrial/Views/baseFrame.h>

class OLEDDisplay;
class OLEDDisplayUiState;
class ReceiverFrame : public BaseFrame
{
public:
    ReceiverFrame(RECEIVER_SETTINGS* state);
    virtual void SetActive(bool isActive) override;
    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override;
    void UpdateRSSI(uint16_t rssia, uint16_t rssib)
    {
        _rssiA = rssia;
        _rssiB = rssib;
    }

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    RECEIVER_SETTINGS* _dataModel;
    uint16_t _rssiA;
    uint16_t _rssiB;
};
