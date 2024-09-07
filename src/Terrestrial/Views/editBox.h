#pragma once
#include <Arduino.h>
#include <Terrestrial/Views/childWidget.h>
#include <string>
#include <functional>

class OLEDDisplay;
class OLEDDisplayUiState;
class EditBox : public ChildWidget
{
public:
    EditBox(uint16_t value, uint16_t min, uint16_t max, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override;
    void OnEditCompeted(std::function<void(EditBox* sender, uint16_t value)> callback)
    {
        _callback = callback;
    }

    void UpdateValue(uint16_t value)
    {
        _value = value;
    }

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Edited(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;

private:
    uint16_t _value;
    uint16_t _minValue;
    uint16_t _maxValue;
    char _editValue[6];
    uint8_t _editWidth;
    int8_t _editedPos;
    bool _isBlink = false;
    uint8_t _blinkTimer;
    std::function<void(EditBox* sender, uint16_t value)> _callback;
};
