#include <Terrestrial/Views/editBox.h>

EditBox::EditBox(IWidgetObserver* parent, uint16_t value, uint16_t min, uint16_t max, uint8_t x, uint8_t y, uint8_t width, uint8_t height)
    : ChildWidget(parent, x, y, width, height), _value(value), _minValue(min), _maxValue(max)
{

}

void EditBox::SetCommand(WIDGET_COMMAND_TYPE command)
{
    if (_state == WIDGET_STATE_TYPE::Active)
    {
        if (command == WIDGET_COMMAND_TYPE::ENTER)
        {
            char format[5] = "%05d";
            if (_maxValue < 10000) format[2] = '4';
            if (_maxValue < 1000) format[2] = '3';
            if (_maxValue < 100) format[2] = '2';
            if (_maxValue < 10) format[2] = '1';
            sprintf(_editValue, format, _value);
            _editWidth = strlen(_editValue) * 16;
            _editedPos = 0;

            _state = WIDGET_STATE_TYPE::Edited;
        }
    }
    else if (_state == WIDGET_STATE_TYPE::Edited)
    {
        if (command == WIDGET_COMMAND_TYPE::INCREMENT)
        {
            if (_isBlink)
            {
                auto v = _editValue[_editedPos];
                ++v;
                if (v > '9') v = '0';
                _editValue[_editedPos] = v;
            }
            else
            {
                ++_editedPos;
                if (_editedPos == strlen(_editValue)) _editedPos = 0;
            }
        }
        else if (command == WIDGET_COMMAND_TYPE::DECREMENT)
        {
            if (_isBlink)
            {
                auto v = _editValue[_editedPos];
                --v;
                if (v < '0') v = '9';
                _editValue[_editedPos] = v;

            }
            else
            {
                --_editedPos;
                if (_editedPos < 0) _editedPos = strlen(_editValue) - 1;
            }
        }
        else if (command == WIDGET_COMMAND_TYPE::ENTER)
        {
            _isBlink = !_isBlink;
            _blinkTimer = 0;
        }
        else if (command == WIDGET_COMMAND_TYPE::CANCEL)
        {
            _state = WIDGET_STATE_TYPE::Active;
            _isBlink = false;

            if (_callback)
            {
                _callback(this, atoi(_editValue));
            }
        }
    }
}

void EditBox::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_10);
    auto tmp = std::to_string(_value);
    display->drawString(_x + x, _y + y, tmp.c_str());
}

void EditBox::Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setFont(ArialMT_Plain_10);
    auto tmp = std::to_string(_value);
    display->drawString(_x + x, _y + y, tmp.c_str());
    display->setColor(INVERSE);
    display->fillRect(_x + x, _y + y + 2, _width, _height);
    display->setColor(WHITE);
}

void EditBox::Edited(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    // Background
    Active(display, state, x, y);

    // Edit field
    display->setColor(BLACK);
    display->fillRect((128 - _editWidth) / 2 + x, 18 + y, _editWidth, 28);
    display->setColor(WHITE);
    display->setFont(ArialMT_Plain_24);
    display->drawString((128 - _editWidth) / 2 + 5 + x, 18 + y, _editValue);
    display->drawRect((128 - _editWidth) / 2 + x, 18 + y, _editWidth, 28);

    // Cursor
    if (_isBlink)
    {
        ++_blinkTimer;
        if (_blinkTimer < 10)
        {
            return;
        }

        if (_blinkTimer == 20)
        {
            _blinkTimer = 0;
        }
    }

    display->setColor(INVERSE);
    display->fillRect((128 - _editWidth) / 2 + x + 5 + _editedPos * 13, 18 + y + 2, 13, 28 - 4);
    display->setColor(WHITE);
}
