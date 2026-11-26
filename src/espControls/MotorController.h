#pragma once
// Standard C++ headers
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <thread>

// ESP-IDF drivers & storage
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "nvs.h"
#include "nvs_flash.h"

// Project headers
#include "pins.h"


/**
 * @brief This is a driver for an individual motor
 */
class Motor {
public:
    pcnt_unit_handle_t encoder;
    /**
     * @brief Constructs a Motor object with the specified hardware configuration and physical parameters.
     * @param pinA GPIO pin for motor input A
     * @param pinB GPIO pin for motor input B
     * @param chanPWM LEDC channel for PWM control
     * @param encoder Pulse counter handle for the motor's encoder
     * @param nvs_key Key used for storing calibration data in NVS
     */
    Motor(gpio_num_t pinA, gpio_num_t pinB, ledc_channel_t chanPWM,
          pcnt_unit_handle_t encoder, const char* nvs_key);

    /**
     * @brief Sets the target speed for the motor in meters per second.
     * @param speed Target speed in m/s. Positive values indicate forward motion, negative values indicate reverse motion.
     */
    void setTargetSpeed(float speed);

    /**
     * @brief Updates the motor's state based on the elapsed time and encoder counts.
     * @param dt Time elapsed since the last update in seconds.
     * @param count Encoder counts since the last update.
     */
    void update(float dt, int count);


    void stop();

    void saveCalibration();
    bool loadCalibration();
    void calibrate();
    /**
     * @brief Sets the PID gains for the motor's speed control.
     * @param kp Proportional gain
     * @param ki Integral gain
     * @param kd Derivative gain (optional, default is 0.0f)
     */
    void setGains(float kp, float ki, float kd = 0.0f);

    float getMeasuredSpeed() const { return this->measured_speed; }
    float getSpeedRatio() const { return this->speed_ratio; }

private:
    void setDirection(bool forward);
    float speedToDuty(float speed) const;

    // Hardware handles
    gpio_num_t pinA;
    gpio_num_t pinB;
    ledc_channel_t chanPWM;
    const char* nvs_key;
    // Physical constants
    float wheel_circumference;
    float speed_ratio = 0.0f; // (rev/s) per duty unit
    const float max_duty = static_cast<float>((1 << pwmconfigs::DUTY_RES) - 1);

    // State variables
    bool is_forward = true;
    float target_speed = 0.0f;
    float measured_speed = 0.0f;

    // Encoder & PID state
    float last_measured_speed = 0.0f;
    float integral = 0.0f;

    // PID gains (output is in duty cycle units)
    float kp = 0.0f;
    float ki = 0.0f;
    float kd = 0.0f;
};


class MotorController {
public:
    Motor& motor1;
    Motor& motor2;
    MotorController(Motor& motor1, Motor& motor2);
    void begin(bool force_calibrate);
};
