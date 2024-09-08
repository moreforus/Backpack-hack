#pragma once
#include <Arduino.h>
#include <IFrame.h>
#include <functional>
#include <vector>
#include <Terrestrial/DataModel/terrestrialState.h>

enum class WIDGET_STATE_TYPE : uint8_t
{
    Preview = 0,
    Active,
    Edited
};

enum class WIDGET_COMMAND_TYPE : uint8_t
{
    NONE = 0,
    INCREMENT,
    DECREMENT,
    ENTER,
    CANCEL,
    BUTTON_PRESS_STARTED
};

class OLEDDisplay;
class OLEDDisplayUiState;
class ChildWidget;
class BaseFrame : public IFrame
{
public:
    virtual void Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) override;
    virtual void SetActive(bool isActive);
    bool IsActive() const
    {
        return _state == WIDGET_STATE_TYPE::Active || _state == WIDGET_STATE_TYPE::Edited;
    }

    virtual void SetCommand(WIDGET_COMMAND_TYPE command) = 0;

    void OnExit(std::function<void(BaseFrame* sender, bool isSave)> callback)
    {
        _onExit = callback;
    }

protected:
    virtual void Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) = 0;
    virtual void Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) = 0;
    virtual void Edited(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y) {};

    WIDGET_STATE_TYPE _state = WIDGET_STATE_TYPE::Preview;
    std::vector<ChildWidget*> _children;
    ChildWidget* _activeWidget = nullptr;
    uint8_t _activeWidgetIndex = -1;
    std::function<void(BaseFrame* sender, bool isSave)> _onExit;
};
