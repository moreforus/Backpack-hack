#include <Terrestrial/Views/button.h>
#include <OLEDDisplay.h>

Button::Button(const std::string& caption, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
    : ChildWidget(x, y, width, height), _caption(caption)
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
    }
}

void Button::DrawRect(OLEDDisplay* display, int16_t x, int16_t y)
{
    uint8_t radius = _width < 16 ? 2 : 3;
    display->drawCircleQuads(_x + x + radius, _y + y + radius, radius, 0x02); // LT
    display->drawLine(_x + x + radius, _y + y, _x + x + _width - radius, _y + y);
    display->drawLine(_x + x + radius, _y + y + _height, _x + x + _width - radius, _y + y + _height);
    display->drawCircleQuads(_x + x + radius, _y + y - radius + _height, radius, 0x04); // LB

    display->drawCircleQuads(_x + x - radius + _width, _y + y + radius, radius, 0x01); // RT
    display->drawLine(_x + x, _y + y + radius, _x + x, _y + y - radius + _height);
    display->drawLine(_x + x + _width, _y + y + radius, _x + x + _width, _y + y - radius + _height);
    display->drawCircleQuads(_x + x - radius + _width, _y + y - radius + _height, radius, 0x08); // RB
}

void Button::Preview(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_16);
    display->drawString(_x + x + 3, _y + y, _caption.c_str());
    DrawRect(display, x, y);
}

void Button::Active(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    uint8_t xAnime = _isPressed ? 1 : 0;
    uint8_t yAnime = _isPressed ? 1 : 0;
    display->setFont(ArialMT_Plain_16);
    display->drawString(_x + x + 3 + xAnime, _y + y + yAnime, _caption.c_str());
    DrawRect(display, x + xAnime, y + yAnime);
    display->setColor(INVERSE);
    display->fillRect(_x + x + xAnime + 1, _y + y + yAnime + 1, _width - 1, _height - 1);
    display->setColor(WHITE);
}
