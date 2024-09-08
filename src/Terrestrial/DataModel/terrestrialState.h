#pragma once
#include <Arduino.h>
#include <common.h>

struct RECEIVER_SETTINGS
{
    uint16_t currentFreq;
};

struct SCANNER_SETTINGS
{
    uint16_t from = 1000;
    uint16_t to = 6000;
    uint16_t filter = 10;
    uint16_t step = 4;
};

struct DEVICE_STATE
{
    uint8_t cpu;
    uint8_t i2c;
    uint8_t battery;
    connectionState_e connectionState;
};

#define RSSI_BUFFER_SIZE (128)

struct TERRESTRIAL_STATE
{
    RECEIVER_SETTINGS receiver;
    SCANNER_SETTINGS scanner;
    DEVICE_STATE device;
    uint16_t rssiA;
    uint16_t rssiB;
    uint16_t rssi[RSSI_BUFFER_SIZE];
};
