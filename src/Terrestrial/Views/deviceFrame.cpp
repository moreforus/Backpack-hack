#include <Terrestrial/Views/deviceFrame.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <string>
#include <common.h>

DeviceFrame::DeviceFrame(DEVICE_STATE* dataModel)
    : _dataModel(dataModel)
{

}

static const char* stateMessage[] = {"Starting", "Binding", "Running", "Update", "ERROR"};

void DeviceFrame::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_16);
    display->drawString(0 + x, 0 + y, "Tasks");
    display->setFont(ArialMT_Plain_10);
    display->drawString(0 + x, 18 + y, "CPU:");
    display->drawString(0 + x, 28 + y, "I2C:");
    display->drawString(0 + x, 38 + y, "Con:");

    auto tmp = std::to_string(_dataModel->cpu0);
    display->drawString((tmp.size() == 1 ? 35 : 30) + x, 18 + y, tmp.c_str());
    tmp = std::to_string(_dataModel->cpu1);
    display->drawString((tmp.size() == 1 ? 55 : 50) + x, 18 + y, tmp.c_str());
    tmp = std::to_string(_dataModel->i2c);
    display->drawString((tmp.size() == 1 ? 34 : 30) + x, 28 + y, tmp.c_str());

    display->drawString(30 + x, 38 + y, stateMessage[_dataModel->connectionState]);
}
