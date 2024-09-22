#pragma once
#include <Terrestrial/Views/baseFrame.h>

class OLEDDisplay;
class OLEDDisplayUiState;
class ScannerFrame : public BaseFrame
{
public:
    ScannerFrame(SCANNER_SETTINGS* dataModel);
    virtual void SetActive(bool isActive) override;
    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override;
    void UpdateRSSI(uint16_t* rssi, const SCANNER_STATE& scannerState)
    {
        _rssi = rssi;
        _scannerState = scannerState;
    }

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    SCANNER_SETTINGS* _dataModel;
    uint16_t* _rssi = nullptr;
    SCANNER_STATE _scannerState;
};
