#include <menuItem.h>
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>

void 
MenuItem::Draw(OLEDDisplay* display,  OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    _func(display, state, x, y);
}
