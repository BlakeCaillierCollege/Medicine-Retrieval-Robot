#include "MotorController.h"

void Motor::setSpeed(int speed) {
    // this function will handle velocities out of range
    SpeedToPWM(speed);
    this->speed = speed;
}
// this stops the motors while keeping the last speed in memory
void Motor::stop() {
    setSpeed(0);
}
void Motor::resume() {
    setSpeed(this->speed);
}

void Motor::SpeedToPWM(int speed) {
    // TODO
    // convert velocity to PWM signal
    // take samples from encoder and get a best fit graph
}


void MotorController::turnLeft(int angle) {
    // 
}

void MotorController::turnRight(int angle) {
    
}
