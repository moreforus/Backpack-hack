#pragma once
#include <Arduino.h>

extern QueueHandle_t commandQueue;
extern QueueHandle_t responseQueue;

void consoleTask(void* params);