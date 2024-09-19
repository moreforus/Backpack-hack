#pragma once
#include <Arduino.h>

enum WORK_MODE_TYPE : uint8_t
{
    NONE = 0,
    RECEIVER,
    SCANNER,
};

typedef struct 
{
    WORK_MODE_TYPE work;
    uint16_t freq;
    uint16_t scannerFrom;
    uint16_t scannerTo;
    uint16_t scannerFilter;
    uint8_t scannerStep;
} TerrestrialCommand_t;

typedef struct 
{
    WORK_MODE_TYPE work;
    uint16_t freq;
    uint16_t rssiA;
    uint16_t rssiB;
    uint8_t antenna;
} TerrestrialResponse_t;
