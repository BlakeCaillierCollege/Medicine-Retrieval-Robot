#pragma once
#include <stdint.h>


class Motor {
private:
    void SpeedToPWM(int speed);
public:
    int speed;
    gpio_num_t pinA;
    gpio_num_t pinB;
    gpio_num_t pinPWM;
    gpio_num_t encoderA;
    gpio_num_t encoderB;
    void setSpeed(int speed);
    void calibrate();
    void pause();
    void resume();
};

class MotorController {
public:
    Motor motor1;
    Motor motor2;
    MotorController(Motor motor1, Motor motor2);
    void turnLeft(int angle);
    void turnRight(int angle);
private:
    int currentSpeed;
};