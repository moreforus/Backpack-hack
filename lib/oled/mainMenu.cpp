#include <mainMenu.h>
#include <SSD1306Wire.h>
#include <functional>
#include <OLEDDisplayUi.h>
#include <menuItem.h>

MainMenu::MainMenu()
{
    _display = new SSD1306Wire(0x3c, SDA, SCL);
    _ui = new OLEDDisplayUi(_display);
}

MainMenu::~MainMenu()
{
    if (_ui)
    {
        delete _ui;
        _ui = nullptr;
    }

    if (_display)
    {
        delete _display;
        _display = nullptr;
    }
}

void 
MainMenu::Init()
{
    _ui->setTargetFPS(25);
    //_ui->setActiveSymbol(activeSymbol);
    //_ui->setInactiveSymbol(inactiveSymbol);
    _ui->setIndicatorPosition(BOTTOM);
    _ui->setIndicatorDirection(LEFT_RIGHT);
    _ui->setFrameAnimation(SLIDE_LEFT);
    _frames.push_back(new MenuItem([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->setFont(ArialMT_Plain_10);
            display->drawString(0 + x, 10 + y, "Arial 10");
            display->setFont(ArialMT_Plain_16);
            display->drawString(0 + x, 20 + y, "Arial 16");
            display->setFont(ArialMT_Plain_24);
            display->drawString(0 + x, 34 + y, "Arial 24");
        }
        ));
    _frames.push_back(new MenuItem([&](OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
        {
            display->setFont(ArialMT_Plain_10);
            display->setTextAlignment(TEXT_ALIGN_LEFT);
            display->drawString(0 + x, 11 + y, "Left aligned (0,10)");
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->drawString(64 + x, 22 + y, "Center aligned (64,22)");
            display->setTextAlignment(TEXT_ALIGN_RIGHT);
            display->drawString(128 + x, 33 + y, "Right aligned (128,33)");
        }
        ));        
    _ui->setFrames(_frames.data(), _frames.size());
    //_ui->setOverlays(overlays, overlaysCount);
    _ui->init();
    _display->flipScreenVertically();
}

void
MainMenu::SetUserAction(uint8_t action)
{
    if (action == 1)
    {
        _ui->nextFrame();
    }
    else if (action == 2)
    {
        _ui->previousFrame();
    }
}

void
MainMenu::Loop()
{
    _ui->update();
}
