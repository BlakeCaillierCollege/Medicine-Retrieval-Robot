#include "TaskManager.h"

void TaskManager::pidTaskTrampoline(void* parameter) {
    static_cast<TaskManager*>(parameter)->pidLoop();
}

void TaskManager::pidLoop() {
    TickType_t xlastwake = xTaskGetTickCount();
    TickType_t xpreviouswake = xlastwake;
    const TickType_t frequency = pdMS_TO_TICKS(20); // 20 ms
    float dt;
    for (;;) {
        PIDSample sample;
        const TickType_t xthiswake = xTaskGetTickCount();
        dt = (xthiswake - xpreviouswake) * portTICK_PERIOD_MS / 1000.0f;
        if (dt > 0.0f) {
            int count1;
            int count2;
            pcnt_unit_get_count(controller_.motor1.getEncoder(), &count1);
            pcnt_unit_get_count(controller_.motor2.getEncoder(), &count2);
            controller_.motor1.update(dt, count1);
            controller_.motor2.update(dt, count2);
        }
        sample.sequence = xthiswake;
        sample.target_speed1 = static_cast<int16_t>(controller_.motor1.getTargetSpeed() * 1000); // Convert to mm/s
        sample.measured_speed1 = static_cast<int16_t>(controller_.motor1.getMeasuredSpeed() * 1000); // Convert to mm/s
        sample.target_speed2 = static_cast<int16_t>(controller_.motor2.getTargetSpeed() * 1000); // Convert to mm/s
        sample.measured_speed2 = static_cast<int16_t>(controller_.motor2.getMeasuredSpeed() * 1000); // Convert to mm/s
        portENTER_CRITICAL(&pid_buffer_lock_);
        pid_buffer_.push(sample);
        portEXIT_CRITICAL(&pid_buffer_lock_);

        xpreviouswake = xthiswake;
        if ((TickType_t)(xthiswake - xlastwake) >= frequency) {
            xlastwake = xthiswake;
        }
        vTaskDelayUntil(&xlastwake, frequency);
    }
}

size_t TaskManager::drainSamples(PIDSample* destination, size_t max_count) {
    portENTER_CRITICAL(&pid_buffer_lock_);
    const size_t drained = pid_buffer_.drain(destination, max_count);
    portEXIT_CRITICAL(&pid_buffer_lock_);
    return drained;
}

esp_err_t TaskManager::begin() {
    if (xPIDTaskHandle_ != nullptr) return ESP_OK;
    const BaseType_t result = xTaskCreate(
        pidTaskTrampoline,
        "PID_Loop",
        4096,
        this,
        3,
        &xPIDTaskHandle_
    );
    if (result != pdPASS) {
        ESP_LOGE("TaskManager", "Failed to create PID control task");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

MotorController& TaskManager::getController() {
    return controller_;
}
