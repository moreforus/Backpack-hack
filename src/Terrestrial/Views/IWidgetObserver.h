#pragma once
#include <Arduino.h>

class ChildWidget;
class IWidgetObserver
{
public:
    virtual void OnAction(ChildWidget* widget, void* data) = 0;
};
