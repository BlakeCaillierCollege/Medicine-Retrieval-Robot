#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MotorController.h"
#include "TaskManager.h"
#include "esp_log.h"

TaskHandle_t xPIDTaskHandle = NULL; 

// Create a new task
void vfeedback(void* pvParameters) {
    auto* controller = static_cast<MotorController*>(pvParameters);
    if (controller == nullptr) {
        vTaskDelete(nullptr);
        return;
    }

    TickType_t xlastwake = xTaskGetTickCount();
    const TickType_t frequency = pdMS_TO_TICKS(10); // 10 ms
    TickType_t xpreviouswake = xlastwake;
    float dt;
    for (;;) {
        dt = (xlastwake - xpreviouswake) * portTICK_PERIOD_MS / 1000.0f; // convert to seconds
        if (dt > 0.0f) {
            int count1;
            int count2;
            pcnt_unit_get_count(controller->motor1.encoder, &count1);
            pcnt_unit_get_count(controller->motor2.encoder, &count2);
            controller->motor1.update(dt, count1);
            controller->motor2.update(dt, count2);
        }
        xpreviouswake = xlastwake;
        vTaskDelayUntil(&xlastwake, frequency);
    }
};


BaseType_t xTaskInit(MotorController* controller) {
    // Create a new task
    BaseType_t res1 = xTaskCreate(
        vfeedback,   // Function pointer to the task
        "PID_Loop",        // Descriptive name (for debugging)
        4096,              // ESP-IDF specifies task stack size in bytes
        controller,        // Motor controller used by the feedback task
        3,                 // Task priority (higher number = higher priority; 0 is Idle)
        &xPIDTaskHandle    // Optional TaskHandle_t pointer (used to suspend/delete later)
    );
    if (res1 != pdPASS) {
        // Handle task creation failure (e.g., log an error)
        ESP_LOGE("TaskManager", "Failed to create PID control task");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}
