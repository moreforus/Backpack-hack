#pragma once

#include <Arduino.h>
#include <vector>
#include <mutex>

enum class IENCODER_STATE : uint8_t
{
    NONE = 0x00,
    PLUS = 0x01,
    MINUS = 0x02,
    BUTTON_PRESS = 0x10,
    PLUS_PRESS_BUTTON = 0x11,
    MINUS_PRESS_BUTTON = 0x12,
    BUTTON_SHORT_PRESS = 0x40,
    BUTTON_LONG_PRESS = 0x80
};

class IncrementalEncoder
{
public:
    IncrementalEncoder();
    ~IncrementalEncoder();
    IENCODER_STATE GetState();
    void Poll(uint32_t now);

private:
    std::mutex _bufferMutex;
    std::vector<IENCODER_STATE> _buffer;
    uint8_t _clk;
    uint8_t _sw;
    uint16_t _swFilter = 0;
    uint32_t _pollTime;
};
