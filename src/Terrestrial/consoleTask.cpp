#include <Terrestrial/consoleTask.h>
#include <string>
#include <Terrestrial/remoteConsole.h>
#include <Terrestrial/userConsole.h>
#include <Terrestrial/workMode.h>

QueueHandle_t commandQueue = nullptr;
QueueHandle_t responseQueue = nullptr;

static void
PushCommandToQueue(const TerrestrialCommand_t& command)
{
    if (command.work != WORK_MODE_TYPE::NONE)
    {
        xQueueSend(commandQueue, (void *) &command, (TickType_t)0);
    }
}

void consoleTask(void* params)
{
    IConsole* _remoteConsole;
    IConsole* _userConsole;

    commandQueue = xQueueCreate(10, sizeof(TerrestrialCommand_t));
    responseQueue = xQueueCreate(10, sizeof(TerrestrialResponse_t));

    TERRESTRIAL_STATE* _state = (TERRESTRIAL_STATE*)params;
    _remoteConsole = new RemoteConsole(921600);
    _remoteConsole->Init();

    _userConsole = new UserConsole(_state);
    _userConsole->Init();

    uint32_t currentTimeMs = 0;

    for (;;)
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);

        auto usStart = micros();
        uint32_t now = usStart / 1000;

        _remoteConsole->Loop();
        auto command = _remoteConsole->GetCommand();
        PushCommandToQueue(command);

        TerrestrialResponse_t response;
        if (xQueueReceive(responseQueue, &response, 0))
        {
            _remoteConsole->SendMessage(response);
            _userConsole->SendMessage(response);
        }
    
        if (currentTimeMs == now)
        {
            continue;
        }

        currentTimeMs = now;
        auto usI2CStart = micros();

        _userConsole->Loop();
        command = _userConsole->GetCommand();
        PushCommandToQueue(command);

        auto usStop = micros();
        uint8_t i2c = (usStop - usI2CStart) / 10;
        _state->device.i2c = i2c > 100 ? 100 : i2c;
        uint8_t cpu = (usStop - usStart) / 10;
        _state->device.cpu0 = cpu > 100 ? 100 : cpu;
    }
}
