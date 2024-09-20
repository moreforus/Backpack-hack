#pragma once

class OLEDDisplay;
class OLEDDisplayUiState;
class IOverlay
{
public:
    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state) = 0;
    virtual void SetData(void* data) = 0;
};
