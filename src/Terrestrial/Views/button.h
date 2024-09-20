#pragma once
#include <Terrestrial/Views/childWidget.h>
#include <string>
#include <functional>

class OLEDDisplay;
class OLEDDisplayUiState;
class Button : public ChildWidget
{
public:
    Button(const std::string& caption, uint8_t x, uint8_t y, uint8_t width, uint8_t height);
    virtual void SetCommand(WIDGET_COMMAND_TYPE command) override;
    void OnButtonPressed(std::function<void()> callback)
    {
        _callback = callback;
    }

protected:
    virtual void Preview(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void Active(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y) override;

private:
    void DrawRect(OLEDDisplay* display, int16_t x, int16_t y);
    std::string _caption;
    bool _isPressed = false;
    std::function<void()> _callback;
};
