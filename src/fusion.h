#pragma once

#include "module_base.h"
#include <Arduino.h>
#include <channels.h>

#define VRX_BOOT_DELAY  1000

#define VRX_UART_BAUD   500000   // fusion uses 500k baud between the ESP8266 and the STM32

class Fusion : public ModuleBase
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    void SendLinkTelemetry(uint8_t *rawCrsfPacket);
    void SendBatteryTelemetry(uint8_t *rawCrsfPacket);
};
