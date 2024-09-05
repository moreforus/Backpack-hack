#pragma once
#include <Arduino.h>
#include <IMenu.h>
#include <vector>

class SSD1306Wire;
class OLEDDisplay;
class OLEDDisplayUi;
class OLEDDisplayUiState;
class IFrame;
class MainMenu final : public IMenu
{
public:
    MainMenu();
    virtual ~MainMenu();
    virtual void Init() override;
    virtual void SetUserAction(uint8_t action) override;
    virtual void Loop() override;

private:
    SSD1306Wire* _display;
    OLEDDisplayUi* _ui;
    std::vector<IFrame*> _frames;
};
