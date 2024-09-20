#pragma once
#include <cstdlib>

class OLEDDisplay;
class OLEDDisplayUiState;
class IFrame
{
public:
    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) = 0;
};
