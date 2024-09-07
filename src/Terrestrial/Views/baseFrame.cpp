#include <Terrestrial\Views\baseFrame.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <Terrestrial/Views/childWidget.h>

void 
BaseFrame::Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    if (_state == WIDGET_STATE_TYPE::Preview)
    {
        // just show
        Preview(display, state, x, y);
    }
    else if (_state == WIDGET_STATE_TYPE::Active)
    {
        // show selected
        Active(display, state, x, y);
    }
    else if (_state == WIDGET_STATE_TYPE::Edited)
    {
        // show edited
        Edited(display, state, x, y);
    }
}

void
BaseFrame::SetActive(bool isActive)
{
    _state = isActive ? WIDGET_STATE_TYPE::Active : WIDGET_STATE_TYPE::Preview;
    if (isActive)
    {
        _activeWidget = _children.size() > 0 ? _children.front() : nullptr;
        _activeWidgetIndex = _children.size() > 0 ? 0 : -1;
        if (_activeWidget)
        {
            _activeWidget->SetActive(true);
        }
    }
}
