#include <iencoder.h>

/*static void IRAM_ATTR OnEncoderChanged()
{

}*/

IncrementalEncoder::IncrementalEncoder()
{
#if defined(IENC_SW) && defined(IENC_DT) && defined(IENC_CLK)
    pinMode(IENC_SW, INPUT_PULLUP);
    pinMode(IENC_DT, INPUT);
    pinMode(IENC_CLK, INPUT);

    _clk = digitalRead(IENC_CLK);
    //attachInterrupt(IENC_CLK, OnEncoderChanged, CHANGE);
#endif
}

IncrementalEncoder::~IncrementalEncoder()
{
#if defined(IENC_CLK)
    //detachInterrupt(IENC_CLK);
#endif
}

IENCODER_STATE
IncrementalEncoder::GetState()
{
    std::lock_guard<std::mutex> _lock(_bufferMutex);
    if (_buffer.size() > 0)
    {
        auto tmp = _buffer.front();
        _buffer.erase(_buffer.begin());
        return tmp;
    }

    return IENCODER_STATE::NONE;
}

void
IncrementalEncoder::Poll(uint32_t now)
{
#if defined(IENC_SW) && defined(IENC_DT) && defined(IENC_CLK)
    auto clk = digitalRead(IENC_CLK);
    auto dt = digitalRead(IENC_DT);
    uint8_t sw = digitalRead(IENC_SW);

    if (now - _pollTime > 0)
    {
        if (sw == LOW)
        {
            ++_swFilter;
            if (_swFilter == 1000)
            {
                std::lock_guard<std::mutex> _lock(_bufferMutex);
                _buffer.push_back(IENCODER_STATE::BUTTON_LONG_PRESS);
            }
            
            if (_swFilter == 10)
            {
                std::lock_guard<std::mutex> _lock(_bufferMutex);
                _buffer.push_back(IENCODER_STATE::BUTTON_START_PRESS);    
            }
        }
        else
        {
            if (_swFilter > 20 && _swFilter < 250)
            {
                std::lock_guard<std::mutex> _lock(_bufferMutex);
                _buffer.push_back(IENCODER_STATE::BUTTON_SHORT_PRESS);            
            }

            _swFilter = 0;
        }
        
        _pollTime = now;
    }

    if (_clk != clk)
    {
        auto data = clk ? ((dt ? IENCODER_STATE::MINUS : IENCODER_STATE::PLUS)) : ((dt ? IENCODER_STATE::PLUS : IENCODER_STATE::MINUS));
        _clk = clk;
        std::lock_guard<std::mutex> _lock(_bufferMutex);
        _buffer.push_back((IENCODER_STATE)((uint8_t)data | (uint8_t)(sw == LOW ? IENCODER_STATE::BUTTON_PRESS : IENCODER_STATE::NONE)));
    }

#endif
}
