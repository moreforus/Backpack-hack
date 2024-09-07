#include <Terrestrial/Views/button.h>
#include <Terrestrial/Views/IWidgetObserver.h>

Button::Button(IWidgetObserver* parent, const std::string& caption, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
    : ChildWidget(parent, x, y, width, height), _caption(caption)
{

}

void Button::SetCommand(WIDGET_COMMAND_TYPE command)
{
    _isPressed = command == WIDGET_COMMAND_TYPE::BUTTON_PRESS_STARTED;
    if (command == WIDGET_COMMAND_TYPE::ENTER)
    {
        _isPressed = false;
        _state = WIDGET_STATE_TYPE::Preview;
        if (_callback)
        {
            _callback();
        }

        _parent->OnAction(this, nullptr);
    }
}

void Button::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_16);
    display->drawString(_x + x + 3, _y + y, _caption.c_str());
    display->drawRect(_x + x, _y + y, _width, _height);
}

void Button::Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    uint8_t xAnime = _isPressed ? 1 : 0;
    uint8_t yAnime = _isPressed ? 1 : 0;
    display->setFont(ArialMT_Plain_16);
    display->drawString(_x + x + 3 + xAnime, _y + y + yAnime, _caption.c_str());
    display->setColor(INVERSE);
    display->fillRect(_x + x + xAnime, _y + y + yAnime, _width, _height);
    display->setColor(WHITE);
}
