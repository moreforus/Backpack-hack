#pragma once
#include <Arduino.h>
#include <Terrestrial/Views/baseFrame.h>
#include <string>

class OLEDDisplay;
class OLEDDisplayUiState;
class IWidgetObserver;
class ChildWidget : public BaseFrame
{
public:
    ChildWidget(IWidgetObserver* parent, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
        : _parent(parent), _x(x), _y(y), _width(width), _height(height)
    {

    }

    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override
    {
        if (_isVisible)
        {
            BaseFrame::Draw(display, state, x, y);
        }
    }

    void SetVisibility(bool isVisible)
    {
        _isVisible = isVisible;
    }

    bool IsVisible() const
    {
        return _isVisible;
    }

    bool IsEdited() const
    {
        return _state == WIDGET_STATE_TYPE::Edited;
    }

protected:
    IWidgetObserver* _parent;
    uint8_t _x;
    uint8_t _y;
    uint8_t _width;
    uint8_t _height;
    bool _isVisible = true;
};
