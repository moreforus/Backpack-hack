#pragma once
#include <cstdlib>

typedef uint16_t frequency_t;

typedef enum
{
    starting,
    binding,
    running,
    wifiUpdate,
    FAILURE_STATES
} connectionState_e;

extern connectionState_e connectionState;
extern unsigned long bindingStart;
