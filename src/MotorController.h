#pragma once
#include <stdint.h>


class Motor {
private:
    void SpeedToPWM(int speed);
public:
int speed;
uint8_t pinA;
uint8_t pinB;
uint8_t pinPWM;
uint8_t encoderA;
uint8_t encoderB;
void setSpeed(int speed);
void stop();
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