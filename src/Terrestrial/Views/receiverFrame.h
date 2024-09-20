#pragma once
#include <Terrestrial/Views/baseFrame.h>

class OLEDDisplay;
class OLEDDisplayUiState;
class ReceiverFrame : public BaseFrame
{
public:
    ReceiverFrame(RECEIVER_SETTINGS* state);
    virtual void SetActive(bool isActive) override;
    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override;
    void UpdateRSSI(const RECEIVER_STATE& recState)
    {
        _rssiA = recState.rssiA;
        _rssiB = recState.rssiB;
    }

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    RECEIVER_SETTINGS* _dataModel;
    uint16_t _rssiA;
    uint16_t _rssiB;
};
