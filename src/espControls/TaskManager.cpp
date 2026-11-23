#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "MotorController.h"

void feedback() {
    // read from the encoders
    // call the PID function
    // set that speed
};

void move() {

};

void test() {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500);
    for (;;) {
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
    
};