#pragma once
#include <Arduino.h>
#include <IFrame.h>
#include <functional>

class OLEDDisplay;
class OLEDDisplayUiState;
class MenuItem : public IFrame
{
public:
    MenuItem(std::function<void(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)> func)
    {
        _func = func;
    }

    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;

private:
    std::function<void(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)> _func;
};
