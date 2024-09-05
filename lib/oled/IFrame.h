#pragma once
#include <Arduino.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>

class IFrame
{
public:
    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) = 0;
};
