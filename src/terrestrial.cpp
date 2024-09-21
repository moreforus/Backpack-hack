#include "terrestrial.h"
#include <SPI.h>
#include "logging.h"
#include <channels.h>
#include <rtc6715.h>
#include <lib_rtc6712.h>
#include <common.h>
#include "config.h"
#include <Terrestrial/consoleTask.h>

#define RSSI_DIFF_BORDER (16)

#define MIN_1G2_FREQ (500)
#define MAX_1G2_FREQ (2500)

#define MIN_5G8_FREQ (4900)
#define MAX_5G8_FREQ (6000)

void
Terrestrial::Init()
{
    ModuleBase::Init();
    auto cfg = _config->GetTerrestrialConfig();
    _state.receiver.currentFreq = cfg->receiverFreq;
    _state.scanner.from = cfg->from;
    _state.scanner.to = cfg->to;
    _state.scanner.filter = cfg->filter;
    _state.scanner.step = cfg->step;

    // common MOSI and CLK pins
    pinMode(PIN_MOSI, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_5G8_CS, INPUT);
    pinMode(PIN_1G2_CS, INPUT);
    pinMode(VIDEO_CTRL, OUTPUT);

    memset(_state.scannerState.rssi, 0, sizeof(_state.scannerState.rssi));

    DBGLN("Terrestrial init complete");

    xTaskCreatePinnedToCore(consoleTask, "consoleTask", 5000, (void*)&_state, 1, NULL, 0);

    _scanner1G2 = new Scanner(rtc6712SetFreq, RSSI_1G2_A, RSSI_1G2_B, RSSI_DIFF_BORDER);
    _scanner5G8 = new Scanner(rtc6715SetFreq, RSSI_5G8_A, RSSI_5G8_B, RSSI_DIFF_BORDER);
    
    EnableSPIMode();
}

void
Terrestrial::EnableSPIMode()
{
    pinMode(PIN_MOSI, OUTPUT);
    pinMode(PIN_CLK, OUTPUT);
    pinMode(PIN_5G8_CS, OUTPUT);
    pinMode(PIN_1G2_CS, OUTPUT);
    
    digitalWrite(PIN_MOSI, LOW);
    digitalWrite(PIN_CLK, LOW);
    digitalWrite(PIN_5G8_CS, HIGH);
    digitalWrite(PIN_1G2_CS, HIGH);

    DBGLN("SPI config complete");
}

void
Terrestrial::SetFreq(frequency_t freq)
{
    if (freq >= MIN_5G8_FREQ)
    {
        rtc6715SetFreq(freq);
        _state.receiver.currentFreq = freq;
    }
    else if (freq < MAX_1G2_FREQ)
    {
        rtc6712SetFreq(freq);
        _state.receiver.currentFreq = freq;
    }
}

void
Terrestrial::SendIndexCmd(uint8_t index)
{
    DBG("Setting index ");
    DBGLN("%x", index);

    _state.receiver.currentFreq = frequencyTable[index];

    SetFreq(_state.receiver.currentFreq);
    SaveConfig();
}

void
Terrestrial::SetWorkMode(WORK_MODE_TYPE mode)
{
    if (mode == NONE)
    {
        return;
    }
    
    _workMode = mode;
    if (_workMode == SCANNER)
    {
        // restart scanning process.
        _scannerAuto = INIT;
    }
    else if (_workMode == RECEIVER)
    {
        SetFreq(_state.receiver.currentFreq);
    }
}

#define DEFAULT_RECEIVER_FILTER (8)

TerrestrialResponse_t
Terrestrial::Receive()
{
    TerrestrialResponse_t response;
    response.work = WORK_MODE_TYPE::NONE;
    ANTENNA_TYPE antenna = ANT_A;
    if (CheckRSSI(antenna, DEFAULT_RECEIVER_FILTER))
    {
        if (antenna != _currentAntenna)
        {
            _currentAntenna = antenna;
            SwitchVideo(_currentAntenna);
        }

        response.work = WORK_MODE_TYPE::RECEIVER;
        response.freq = _state.receiver.currentFreq;
        response.rssiA = _state.receiverState.rssiA;
        response.rssiB = _state.receiverState.rssiB;
        response.antenna = _currentAntenna;
    }

    return response;
}

TerrestrialResponse_t 
Terrestrial::ScannerMeasure(Scanner* scanner)
{
    TerrestrialResponse_t response;
    response.work = WORK_MODE_TYPE::NONE;
    if (scanner->MeasureRSSI())
    {
        response.work = WORK_MODE_TYPE::SCANNER;
        response.freq = scanner->GetFreq();
        response.rssiA = scanner->GetRssiA();
        response.rssiB = scanner->GetRssiB();
        response.antenna = 0;
    }

    return response;
}

void
Terrestrial::PrepareBufferForDraw1G2()
{
    uint16_t from = MIN_1G2_FREQ;
    uint16_t to = MAX_1G2_FREQ;
    if (_state.scanner.from < MAX_1G2_FREQ)
    {
        from = _state.scanner.from;
    }
    
    if (_state.scanner.to < MAX_1G2_FREQ)
    {
        to = _state.scanner.to;
    }

    auto count = (to - from) / _scannerStep;
    double rt = ((double)(RSSI_BUFFER_SIZE - 1) / 2) / count;
    uint8_t x = (_scanner1G2->GetFreq() - from) * rt / _scannerStep;
    if (x < RSSI_BUFFER_SIZE)
    {
        if (x != _preX1g2)
        {
            _preX1g2 = x;
            _state.scannerState.rssi[_preX1g2] = 0;
            if (x < RSSI_BUFFER_SIZE / 2 - 1)
            {
                _state.scannerState.rssi[_preX1g2 + 1] = 0;
            }
        }

        uint16_t rssi = _scanner1G2->GetMaxRssi() * _scale1G2;
        if (rssi > _state.scannerState.rssi[x])
        {
            _state.scannerState.rssi[x] = rssi;
        }
    }
}

void
Terrestrial::PrepareBufferForDraw5G8()
{
    uint16_t from = MIN_5G8_FREQ;
    uint16_t to = MAX_5G8_FREQ;
    if (_state.scanner.from >= MIN_5G8_FREQ)
    {
        from = _state.scanner.from;
    }
    
    if (_state.scanner.to < MAX_5G8_FREQ)
    {
        to = _state.scanner.to;
    }

    auto count = (to - from) / _scannerStep;
    double rt = ((double)(RSSI_BUFFER_SIZE - 1) / 2) / count;
    uint8_t x = RSSI_BUFFER_SIZE / 2 + ((_scanner5G8->GetFreq() - from) * rt / _scannerStep);
    if (x < RSSI_BUFFER_SIZE)
    {
        if (x != _preX5g8)
        {
            _preX5g8 = x;
            _state.scannerState.rssi[_preX5g8] = 0;
            if (x < RSSI_BUFFER_SIZE - 1)
            {
                _state.scannerState.rssi[_preX5g8 + 1] = 0;
            }
        }

        uint16_t rssi = _scanner5G8->GetMaxRssi() * _scale5G8;
        if (rssi > _state.scannerState.rssi[x])
        {
            _state.scannerState.rssi[x] = rssi;
        }
    }
}

void
Terrestrial::Work()
{
    if (_workMode == RECEIVER)
    {
        auto response = Receive();
        if (response.work == WORK_MODE_TYPE::RECEIVER)
        {
            xQueueSend(responseQueue, (void*)&response, 0);
        }
    }
    else if (_workMode == SCANNER)
    {
        switch (_scannerAuto)
        {
            case INIT:
                _scanner1G2->Init(_minScanner1G2Freq, _maxScanner1G2Freq);
                _scanner5G8->Init(_minScanner5G8Freq, _maxScanner5G8Freq);
                _scannerAuto = SET_FREQ_1G2;
                _isScalingCompleted = false;
                _preX1g2 = 0;
                _preX5g8 = RSSI_BUFFER_SIZE / 2;
                _state.scannerState.rssi[_preX1g2] = 0;
                _state.scannerState.rssi[_preX5g8] = 0;
            break;
            case SET_FREQ_1G2:
                _scanner1G2->SetFreq(_scannerFilter);
                _scannerAuto = SET_FREQ_5G8;
            break;

            case SET_FREQ_5G8:
                _scanner5G8->SetFreq(_scannerFilter);
                _scannerAuto = MEASURE;
            break;

            case MEASURE:
                auto response = ScannerMeasure(_scanner1G2);
                if (response.work == WORK_MODE_TYPE::SCANNER)
                {
                    xQueueSend(responseQueue, (void*)&response, 0);

                    PrepareBufferForDraw1G2();

                    if (_scanner1G2->IncrementFreq(_scannerStep))
                    {
                        auto state1g2 = _scanner1G2->GetRssiState();
                        _state.scannerState.SetMaxFreq1G2(state1g2.freqForMaxValue);
                        if (!_isScalingCompleted)
                        {
                            _isScalingCompleted = true;

                            auto state5g8 = _scanner5G8->GetRssiState();
                            _scale1G2 = 1.0;
                            _scale5G8 = 1.0;
                            if (state1g2.rssiMin > state5g8.rssiMin)
                            {
                                _scale1G2 = (double)state5g8.rssiMin / state1g2.rssiMin;
                                _scale5G8 = 1.0;
                            }
                            else if (state1g2.rssiMin < state5g8.rssiMin)
                            {
                                _scale5G8 = (double)state1g2.rssiMin / state5g8.rssiMin;
                                _scale1G2 = 1.0;
                            }
                        }

                        _scanner1G2->ResetScannerRssiState();
                    }
                }

                response = ScannerMeasure(_scanner5G8);
                if (response.work == WORK_MODE_TYPE::SCANNER)
                {
                    xQueueSend(responseQueue, (void*)&response, 0);
                    PrepareBufferForDraw5G8();

                    if (_scanner5G8->IncrementFreq(_scannerStep))
                    {
                        auto state5g8 = _scanner5G8->GetRssiState();
                        _state.scannerState.SetMaxFreq5G8(state5g8.freqForMaxValue);
                        _scanner5G8->ResetScannerRssiState();
                    }

                    _scannerAuto = SET_FREQ_1G2;
                }
                
            break;
        }
    }
}

void Terrestrial::SaveConfig()
{
    auto cfg = _config->GetTerrestrialConfig();
    cfg->receiverFreq = _state.receiver.currentFreq;
    cfg->from = _state.scanner.from;
    cfg->to = _state.scanner.to;
    cfg->filter = _state.scanner.filter;
    cfg->step = _state.scanner.step;
    _config->CommitTerrestrial();
}

void 
Terrestrial::Loop(uint32_t now)
{
    auto usStart = micros();
    ModuleBase::Loop(now);

    TerrestrialCommand_t cmd;
    if (xQueueReceive(commandQueue, &cmd, 0))
    {
        if (cmd.work == WORK_MODE_TYPE::RECEIVER)
        {
            _state.receiver.currentFreq = cmd.freq;
            SetWorkMode(cmd.work);
            SaveConfig();
        }
        else if (cmd.work == WORK_MODE_TYPE::SCANNER)
        {
            uint16_t minFreq = cmd.scannerFrom;
            uint16_t maxFreq = cmd.scannerTo;
            _minScanner1G2Freq = 0;
            _maxScanner1G2Freq = 0;
            _minScanner5G8Freq = 0;
            _maxScanner5G8Freq = 0;
            if (minFreq <= MAX_1G2_FREQ)
            {
                _minScanner1G2Freq = minFreq;
                _maxScanner1G2Freq = MAX_1G2_FREQ;

                if (maxFreq > MIN_5G8_FREQ)
                {
                    _minScanner5G8Freq = MIN_5G8_FREQ;
                    _maxScanner5G8Freq = MAX_5G8_FREQ;
                }
            }

            if (maxFreq <= MAX_1G2_FREQ)
            {
                _maxScanner1G2Freq = maxFreq;
            }

            if (minFreq >= MIN_5G8_FREQ && minFreq <= MAX_5G8_FREQ)
            {
                _minScanner5G8Freq = minFreq;
                _maxScanner5G8Freq = MAX_5G8_FREQ;
            }

            if (maxFreq >= MIN_5G8_FREQ && maxFreq <= MAX_5G8_FREQ)
            {
                _maxScanner5G8Freq = maxFreq;
            }

            _state.scanner.from = minFreq;
            _state.scanner.to = maxFreq;
            _scannerStep = cmd.scannerStep;
            _scannerFilter = cmd.scannerFilter;
            _state.scanner.step = _scannerStep;
            _state.scanner.filter = _scannerFilter;
            SetWorkMode(cmd.work);
            SaveConfig();            
        }
    }

    if (_currentTimeMs == now)
    {
        return;
    }

    _currentTimeMs = now;
    auto usStop = micros();
    Work();

    usStop = micros();
    uint8_t cpu = (usStop - usStart) / 10;
    _state.device.cpu1 = cpu > 100 ? 100 : cpu;
    _state.device.connectionState = connectionState;
}

bool 
Terrestrial::CheckRSSI(ANTENNA_TYPE& antenna, uint16_t filterInitCounter)
{
    static uint16_t filter = 0;
    static uint32_t rssiASum = 0;
    static uint32_t rssiBSum = 0;

    if (filter == 0) filter = filterInitCounter;

    if (_state.receiver.currentFreq >= MIN_5G8_FREQ)
    {
        rssiASum += analogRead(RSSI_5G8_A);
        rssiBSum += analogRead(RSSI_5G8_B);
    }
    else if (_state.receiver.currentFreq < MAX_1G2_FREQ)
    {
        rssiASum += analogRead(RSSI_1G2_A);
        rssiBSum += analogRead(RSSI_1G2_B);
    }

    if (--filter == 0)
    {
        _state.receiverState.rssiA = rssiASum / filterInitCounter;
        _state.receiverState.rssiB = rssiBSum / filterInitCounter;

        if (_state.receiverState.rssiA - _state.receiverState.rssiB > RSSI_DIFF_BORDER)
        {
            antenna = ANT_A;
        }
        else if (_state.receiverState.rssiB - _state.receiverState.rssiA > RSSI_DIFF_BORDER)
        {
            antenna = ANT_B;
        }

        rssiASum = 0;
        rssiBSum = 0;

        return true;
    }
 
    return false;
}

void
Terrestrial::SwitchVideo(ANTENNA_TYPE antenna)
{
    digitalWrite(VIDEO_CTRL, (antenna == ANT_A ? LOW : HIGH));
}
