#pragma once
#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MotorController.h"
#include "esp_log.h"
#include "TelemetryBuffer.h"
#include "esp_timer.h"
class TaskManager {
public:
    explicit TaskManager(MotorController& controller) : controller_(controller) {}
    esp_err_t begin();
    size_t drainSamples(PIDSample* destination, size_t max_count);
    MotorController& getController();
private:
    TaskHandle_t xPIDTaskHandle_ = nullptr;
    portMUX_TYPE pid_buffer_lock_ = portMUX_INITIALIZER_UNLOCKED;
    PIDBuffer pid_buffer_;
    MotorController& controller_;
    void pidLoop();
    static void pidTaskTrampoline(void* parameter);
};
