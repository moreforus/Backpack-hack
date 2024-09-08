#include <Terrestrial/Views/scannerFrame.h>
#include <string>
#include <Terrestrial\Views\button.h>
#include <Terrestrial\Views\editBox.h>

ScannerFrame::ScannerFrame(SCANNER_SETTINGS* dataModel)
    : _dataModel(dataModel)
{
    auto edit = new EditBox(dataModel->from, 900, 6000, 30, 18, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->from = value;
        sender->UpdateValue(value);
    });
    _children.push_back(edit);

    edit = new EditBox(dataModel->to, 900, 6000, 30, 28, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->to = value;
        sender->UpdateValue(value);
    });
    _children.push_back(edit);

    edit = new EditBox(dataModel->step, 1, 99, 30, 38, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->step = value;
        sender->UpdateValue(value);
    });
    _children.push_back(edit);

    edit = new EditBox(dataModel->filter, 1, 200, 30, 48, 30, 10);
    edit->OnEditCompeted([&](EditBox* sender, uint16_t value)
    {
        _dataModel->filter = value;
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

void ScannerFrame::SetActive(bool isActive)
{
    _children[4]->SetVisibility(isActive);
    _children[5]->SetVisibility(isActive);

    BaseFrame::SetActive(isActive);
}

void ScannerFrame::SetCommand(WIDGET_COMMAND_TYPE command)
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

void ScannerFrame::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "Scanner");

    if (_rssi != nullptr)
    {
        for(int i = 0; i < RSSI_BUFFER_SIZE; ++i)
        {
            uint8_t height = (64.0 / 2048) * _rssi[i];
            display->drawLine(i + x, 64 - height + y, i + x, 64 + y);
        }
    }
}

void ScannerFrame::Active(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "Scanner");

    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 18 + y, "From:");
    display->drawString(0 + x, 28 + y, "To:");
    display->drawString(0 + x, 38 + y, "Step:");
    display->drawString(0 + x, 48 + y, "Filter:");

    for(int i = 0; i < _children.size(); ++i)
    {
        _children[i]->Draw(display, state, x, y);
    }
}
