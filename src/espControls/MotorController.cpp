#include "MotorController.h"
#include "driver/gpio.h"
MotorController::MotorController(Motor motor1, Motor motor2)
    : motor1(motor1), motor2(motor2), currentSpeed(0) {}

void Motor::setSpeed(int speed) {
    if (speed > 0) {
        gpio_set_level(pinA, 1);
        gpio_set_level(pinB, 0);
    } else if (speed < 0) {
        gpio_set_level(pinA, 0);
        gpio_set_level(pinB, 1);
    } else {
        gpio_set_level(pinA, 0);
        gpio_set_level(pinB, 0);
    }
    this->speed = speed;
}
// this stops the motors while keeping the last speed in memory
void Motor::pause() {
    setSpeed(0);
}
void Motor::resume() {
    setSpeed(this->speed);
}

void Motor::calibrate() {
    // read multiple samples from the encoder
    // store the PWM and encoder voltage to compare
}

void Motor::SpeedToPWM(int speed) {
    for (int i = 0; i < 255; i++) {
        
    }
    // TODO
    // convert velocity to PWM signal
    // take samples from encoder and get a best fit graph
}


void MotorController::turnLeft(int angle) {
    // 
}

void MotorController::turnRight(int angle) {
    
}
