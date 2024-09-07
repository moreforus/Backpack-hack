#include <Terrestrial/Views/deviceFrame.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>
#include <string>

DeviceFrame::DeviceFrame(DEVICE_STATE* dataModel)
    : _dataModel(dataModel)
{

}

 void DeviceFrame::Preview(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
 {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 0 + y, "Tasks");
            display->setFont(ArialMT_Plain_10);
            display->drawString(0 + x, 18 + y, "CPU:");
            display->drawString(0 + x, 28 + y, "I2C:");

            auto tmp = std::to_string(_dataModel->cpu);
            display->drawString((tmp.size() == 1 ? 35 : 30) + x, 18 + y, tmp.c_str());
            tmp = std::to_string(_dataModel->i2c);
            display->drawString((tmp.size() == 1 ? 34 : 30) + x, 28 + y, tmp.c_str());
 }
