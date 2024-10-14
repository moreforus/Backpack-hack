#include "terrestrial.h"
#include <SPI.h>
#include "logging.h"
#include <channels.h>
#include <rtc6715.h>
#include <lib_rtc6712.h>
#include <common.h>
#include "config.h"
#include <Terrestrial/scanner.h>
#include <Terrestrial/consoleTask.h>
#include <Terrestrial/receiversParam.h>
#include <Terrestrial/receiver.h>

#define RSSI_DIFF_BORDER (16)

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
    pinMode(VIDEO_CTRL_5G8, OUTPUT);
    pinMode(VIDEO_CTRL_1G2, OUTPUT);

    xTaskCreatePinnedToCore(consoleTask, "consoleTask", 5000, (void*)&_state, 1, NULL, 0);

    _scanner1G2 = new Scanner(rtc6712SetFreq, RSSI_1G2_A, RSSI_1G2_B, RSSI_DIFF_BORDER, MIN_1G2_FREQ, MAX_1G2_FREQ);
    _scanner5G8 = new Scanner(rtc6715SetFreq, RSSI_5G8_A, RSSI_5G8_B, RSSI_DIFF_BORDER, MIN_5G8_FREQ, MAX_5G8_FREQ);
    _receiver = new Receiver(RSSI_1G2_A, RSSI_1G2_B, VIDEO_CTRL_1G2, RSSI_5G8_A, RSSI_5G8_B, VIDEO_CTRL_5G8, RSSI_DIFF_BORDER);
    _receiver->SetFreq(_state.receiver.currentFreq);
    
    EnableSPIMode();
    DBGLN("Terrestrial init complete");
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
    _receiver->SetFreq(freq);
    _state.receiver.currentFreq = freq;
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
    if (_receiver->MeasureRSSI(antenna, DEFAULT_RECEIVER_FILTER))
    {
        _state.receiverState.rssiA = _receiver->GetRssiA();
        _state.receiverState.rssiB = _receiver->GetRssiB();
        if (antenna != _currentAntenna)
        {
            _currentAntenna = antenna;
            _receiver->SwitchVideo(_currentAntenna);
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
Terrestrial::Work()
{
    if (_workMode == RECEIVER)
    {
        auto response = Receive();
        if (response.work == WORK_MODE_TYPE::RECEIVER)
        {
            response.command = _currentCommand;
            xQueueSend(responseQueue, (void*)&response, 0);
        }
    }
    else if (_workMode == SCANNER)
    {
        switch (_scannerAuto)
        {
            case INIT:
                _scanner1G2->SetBorders(_minScanner1G2Freq, _maxScanner1G2Freq);
                _scanner5G8->SetBorders(_minScanner5G8Freq, _maxScanner5G8Freq);
                _isScalingCompleted = false;
                _scannerAuto = SET_FREQ_1G2;
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
                    if (_scanner1G2->IncrementFreq(_scannerStep))
                    {
                        auto state1g2 = _scanner1G2->GetRssiState();
                        _state.scannerState.SetMaxFreq1G2(state1g2.freqForMaxValue);
                        if (!_isScalingCompleted)
                        {
                            _isScalingCompleted = true;

                            auto state5g8 = _scanner5G8->GetRssiState();
                            auto scale1G2 = 1.0;
                            auto scale5G8 = 1.0;
                            if (state1g2.rssiMin > state5g8.rssiMin)
                            {
                                scale1G2 = (double)state5g8.rssiMin / state1g2.rssiMin;
                            }
                            else if (state1g2.rssiMin < state5g8.rssiMin)
                            {
                                scale5G8 = (double)state1g2.rssiMin / state5g8.rssiMin;
                            }

                            _state.scannerState.scale1G2 = scale1G2 > 0.0 ? scale1G2 : 1.0;
                            _state.scannerState.scale5G8 = scale5G8 > 0.0 ? scale5G8 : 1.0;
                        }

                        _scanner1G2->ResetScannerRssiState();
                    }

                    response.command = _currentCommand;
                    response.scannerState = _state.scannerState;
                    xQueueSend(responseQueue, (void*)&response, 0);
                }

                response = ScannerMeasure(_scanner5G8);
                if (response.work == WORK_MODE_TYPE::SCANNER)
                {
                    if (_scanner5G8->IncrementFreq(_scannerStep))
                    {
                        auto state5g8 = _scanner5G8->GetRssiState();
                        _state.scannerState.SetMaxFreq5G8(state5g8.freqForMaxValue);
                        _scanner5G8->ResetScannerRssiState();
                    }

                    response.command = _currentCommand;
                    response.scannerState = _state.scannerState; 
                    xQueueSend(responseQueue, (void*)&response, 0);
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
            _currentCommand = cmd;
            _state.receiver.currentFreq = cmd.freq;
            SetWorkMode(cmd.work);
            SaveConfig();
        }
        else if (cmd.work == WORK_MODE_TYPE::SCANNER)
        {
            _currentCommand = cmd;
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

    Work();

    uint8_t cpu = (micros() - usStart) / 10;
    _state.device.cpu1 = cpu > 100 ? 100 : cpu;
    _state.device.connectionState = connectionState;
}
