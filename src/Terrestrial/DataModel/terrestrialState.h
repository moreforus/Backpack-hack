#pragma once
#include <Arduino.h>

struct RECEIVER_SETTINGS
{
    uint16_t currentFreq;
};

struct SCANNER_SETTINGS
{
    uint16_t from;
    uint16_t to;
    uint16_t step;
    uint16_t filter;

};

struct DEVICE_STATE
{
    uint8_t cpu;
    uint8_t i2c;
    uint8_t battery;
};

struct TERRESTRIAL_STATE
{
    RECEIVER_SETTINGS receiver;
    SCANNER_SETTINGS scanner;
    DEVICE_STATE device;
    uint16_t rssiA;
    uint16_t rssiB;
    uint16_t rssi[128];
};
