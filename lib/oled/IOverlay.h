#pragma once
#include <Arduino.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>

class IOverlay
{
public:
    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state) = 0;
    virtual void SetData(void* data) = 0;
};
