#include "channels.h"

uint16_t GetFrequency(uint8_t index)
{
    if (index < MAX_CHANNELS)
        return frequencyTable[index];
    else
        return 0;
}

uint8_t GetBand(uint8_t index)
{
    return index / 8 + 1;
}

uint8_t GetChannel(uint8_t index)
{
    return (index % 8) + 1;
}