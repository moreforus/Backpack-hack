#include <Terrestrial/Views/receiverFrame.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <string>
#include <Terrestrial\Views\button.h>
#include <Terrestrial\Views\editBox.h>

ReceiverFrame::ReceiverFrame(RECEIVER_SETTINGS* dataModel)
    : _dataModel(dataModel)
{
    auto edit = new EditBox(this, dataModel->freq, 900, 6000, 30, 18, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->freq = value;
        sender->UpdateValue(value);
    });
    _children.push_back(edit);

    auto btn = new Button(this, "Ok", 100, 47, 26, 16);
    btn->SetVisibility(false);
    btn->OnButtonPressed([&]()
    {
        if (_onExit)
        {
            _onExit(this);
        }
    });
    _children.push_back(btn);

    btn = new Button(this, "X", 112, 0, 16, 16);
    btn->SetVisibility(false);
    _children.push_back(btn);
}

void ReceiverFrame::SetActive(bool isActive)
{
    _children[1]->SetVisibility(isActive);
    _children[2]->SetVisibility(isActive);

    BaseFrame::SetActive(isActive);
}

void ReceiverFrame::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "Receiver");
   
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 18 + y, "Freq:");

    for(int i = 0; i < _children.size(); ++i)
    {
        _children[i]->Draw(display, state, x, y);
    }
}

void ReceiverFrame::Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "Receiver");

    for(int i = 0; i < _children.size(); ++i)
    {
        _children[i]->Draw(display, state, x, y);
    }
        
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 18 + y, "Freq:");

    //auto tmp = std::to_string(_dataModel->freq);
    //display->drawString(30 + x, 18 + y, tmp.c_str());
}

void ReceiverFrame::SetCommand(WIDGET_COMMAND_TYPE command)
{
    if (_activeWidgetIndex > -1)
    {
        if (_activeWidget->IsEdited())
        {
            _activeWidget->SetCommand(command);
        }
        else
        {
        if (command == WIDGET_COMMAND_TYPE::INCREMENT)
        {
            _activeWidget->SetActive(false);
            if (_activeWidgetIndex < _children.size() - 1)
            {
                ++_activeWidgetIndex;
            }
            else
            {
                _activeWidgetIndex = 0;
            }
            
            _activeWidget = _children[_activeWidgetIndex];
            _activeWidget->SetActive(true);
        }
        else if (command == WIDGET_COMMAND_TYPE::DECREMENT)
        {
            _activeWidget->SetActive(false);
            if (_activeWidgetIndex > 0)
            {
                --_activeWidgetIndex;
            }
            else
            {
                _activeWidgetIndex = _children.size() - 1;
            }

            _activeWidget = _children[_activeWidgetIndex];
            _activeWidget->SetActive(true);
        }
        else 
        {
            _activeWidget->SetCommand(command);
        }
        }
    }

    if (command == WIDGET_COMMAND_TYPE::CANCEL)
    {

    }
}

void ReceiverFrame::OnAction(ChildWidget* widget, void* data)
{

}