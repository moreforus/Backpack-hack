#pragma once
#include <cstdlib>
#include <common.h>
#include <Terrestrial/DataModel/terrestrialState.h>

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
    TerrestrialCommand_t command;
    SCANNER_STATE scannerState;
    
    uint16_t inline GetMaxRssi() const
    {
        return rssiA > rssiB ? rssiA : rssiB;
    }
} TerrestrialResponse_t;
