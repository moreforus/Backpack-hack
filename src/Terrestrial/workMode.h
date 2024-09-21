#pragma once

enum WORK_MODE_TYPE : uint8_t
{
    NONE = 0,
    RECEIVER,
    SCANNER,
};

typedef struct 
{
    WORK_MODE_TYPE work;
    frequency_t freq;
    frequency_t scannerFrom;
    frequency_t scannerTo;
    uint16_t scannerFilter;
    frequency_t scannerStep;
} TerrestrialCommand_t;

typedef struct 
{
    WORK_MODE_TYPE work;
    frequency_t freq;
    uint16_t rssiA;
    uint16_t rssiB;
    uint8_t antenna;
} TerrestrialResponse_t;
