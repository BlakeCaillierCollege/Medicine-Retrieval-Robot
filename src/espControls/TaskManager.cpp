#include "TaskManager.h"

void TaskManager::pidTaskTrampoline(void* parameter) {
    static_cast<TaskManager*>(parameter)->pidLoop();
}

void TaskManager::pidLoop() {

    // == freeRTOS task timer variables ==
    TickType_t first_tick = xTaskGetTickCount();
    TickType_t last_wake = first_tick;
    const TickType_t frequency = pdMS_TO_TICKS(20); // 20 ms

    // == variables to be used in calculations ==
    uint64_t previous_time = esp_timer_get_time();
    float dt;
    PIDSample sample;
    int count1;
    int count2;

    for (;;) {
        // == get and clear the encoder counts, ==
        // == as well as resetting the timer for the next reading == 
        pcnt_unit_get_count(controller_.motor1.getEncoder(), &count1);
        pcnt_unit_get_count(controller_.motor2.getEncoder(), &count2);
        pcnt_unit_clear_count(controller_.motor1.getEncoder());
        pcnt_unit_clear_count(controller_.motor2.getEncoder());
        int64_t start_time = esp_timer_get_time();
        previous_time = start_time;

        // == perform and apply the PID calculations ==
        dt = static_cast<float>(start_time - previous_time) / 1'000'000.0f; // Convert microseconds to seconds
        controller_.motor1.update(dt, count1);
        controller_.motor2.update(dt, count2);

        // == add the data into the buffer sample to be sent over wifi ==
        sample.sequence = static_cast<uint32_t>(last_wake - start_time); // use tick since start as the sequence
        sample.target_speed1 = static_cast<int16_t>(controller_.motor1.getTargetSpeed() * 1000); // Convert to mm/s
        sample.measured_speed1 = static_cast<int16_t>(controller_.motor1.getMeasuredSpeed() * 1000); // Convert to mm/s
        sample.target_speed1 = static_cast<int16_t>(controller_.motor2.getTargetSpeed() * 1000); // Convert to mm/s
        sample.measured_speed2 = static_cast<int16_t>(controller_.motor2.getMeasuredSpeed() * 1000); // Convert to mm/s
        
        // safely push the sample to the circular buffer
        portENTER_CRITICAL(&pid_buffer_lock_);
        pid_buffer_.push(sample);
        portEXIT_CRITICAL(&pid_buffer_lock_);

        // wait until the next task time
        vTaskDelayUntil(&last_wake, frequency);
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
