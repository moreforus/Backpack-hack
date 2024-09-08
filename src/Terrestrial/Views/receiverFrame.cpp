#include <Terrestrial/Views/receiverFrame.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <string>
#include <Terrestrial\Views\button.h>
#include <Terrestrial\Views\editBox.h>

ReceiverFrame::ReceiverFrame(RECEIVER_SETTINGS* dataModel)
    : _dataModel(dataModel)
{
    auto edit = new EditBox(dataModel->currentFreq, 900, 6000, 30, 18, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->currentFreq = value;
        sender->UpdateValue(value);
    });
    _children.push_back(edit);

    auto btn = new Button("Ok", 100, 47, 26, 16);
    btn->SetVisibility(false);
    btn->OnButtonPressed([&]()
    {
        if (_onExit)
        {
            _onExit(this, true);
        }
    });
    _children.push_back(btn);

    btn = new Button("X", 112, 0, 16, 16);
    btn->SetVisibility(false);
    btn->OnButtonPressed([&]()
    {
        if (_onExit)
        {
            _onExit(this, false);
        }
    });
    _children.push_back(btn);
}

void ReceiverFrame::SetActive(bool isActive)
{
    if (isActive)
    {
        ((EditBox*)_children[0])->UpdateValue(_dataModel->currentFreq);
    }

    _children[1]->SetVisibility(isActive);
    _children[2]->SetVisibility(isActive);

    BaseFrame::SetActive(isActive);
}

void ReceiverFrame::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "VRx");
   
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    auto tmp = std::to_string(_dataModel->currentFreq);
    display->drawString(64 + x, 18 + y, tmp.c_str());

    uint8_t height = 64.0 / 2048 * rssiA;
    display->fillRect(0 + x, 64 - height + y, 20, height);
    height = 64.0 / 2048 * rssiB;
    display->fillRect(108 + x, 64 - height + y, 20, height);
}

void ReceiverFrame::Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
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
