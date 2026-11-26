#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h" // Changed \ to /
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <MotorController.h>

// Optional: Expose the handle if other files need to suspend/notify this task
extern TaskHandle_t xPIDTaskHandle;

// Prototype for the task function and its initializer
void vfeedback(void *pvParameters);
BaseType_t xTaskInit(MotorController* controller);
