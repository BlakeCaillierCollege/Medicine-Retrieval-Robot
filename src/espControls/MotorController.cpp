#include "MotorController.h"
#include "esp_log.h"


Motor::Motor(gpio_num_t pinA, gpio_num_t pinB, ledc_channel_t chanPWM,
          pcnt_unit_handle_t encoder, const char* nvs_key)
    : encoder(encoder), pinA(pinA), pinB(pinB), chanPWM(chanPWM),
      nvs_key(nvs_key), wheel_circumference(0.044f) {
}

void Motor::update(float dt, int count) {
    if (dt <= 0.0f) return;
    // resets the count to 0 for the next measurement
    pcnt_unit_clear_count(this->encoder);
    // convert counts to revolutions
    float revolutions = static_cast<float>(count) / hardware::ENCODER_COUNTS_PER_REVOLUTION;
    this->measured_speed = (revolutions * this->wheel_circumference) / dt;

    // 2. Clean stop when commanded to 0 m/s
    if (this->target_speed == 0.0f && std::abs(this->measured_speed) < 0.01f) {
        stop();
        return;
    }

    // 3. Calculate PID terms (in m/s)
    float error = this->target_speed - this->measured_speed;

    // Clamp integral so it cannot exceed max duty cycle (prevents windup)
    if (this->ki > 0.0f) {
        float max_integral = this->max_duty / this->ki;
        this->integral = std::clamp(this->integral + (error * dt), -max_integral, max_integral);
    }

    // Derivative on measurement (avoids derivative kick on sudden speed changes)
    float derivative = -(this->measured_speed - this->last_measured_speed) / dt;
    this->last_measured_speed = this->measured_speed;

    // 4. Combine Feedforward + PID output (in signed duty units)
    float ff_duty = speedToDuty(this->target_speed);
    float pid_duty = (this->kp * error) + (this->ki * this->integral) + (this->kd * derivative);
    float total_duty = ff_duty + pid_duty;

    // 5. Apply direction and clamped PWM duty cycle
    setDirection(total_duty >= 0.0f);
    // clamping is handled in analogWrite
    analogWrite(this->chanPWM, std::abs(total_duty));
}

void Motor::calibrate() {
    setDirection(true);
    float sum_x = 0.0f;
    float sum_y = 0.0f;
    float sum_xy = 0.0f;
    float sum_xx = 0.0f;
    size_t valid_samples = 0;
    const size_t n = 50;
    int max_duty = (1 << pwmconfigs::DUTY_RES) - 1;
    int step = max_duty / n;
    int counts = 0;
    int max_abs_counts = 0;

    for (size_t i = 1; i <= n; i++) {
        int duty = step * i;
        analogWrite(this->chanPWM, duty);

        // 1. Let the motor reach steady-state speed before measuring
        std::this_thread::sleep_for(std::chrono::milliseconds(150));

        // 2. Measure counts over a clean 100ms window
        pcnt_unit_clear_count(this->encoder);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        pcnt_unit_get_count(this->encoder, &counts);
        int abs_counts = std::abs(counts);
        max_abs_counts = std::max(max_abs_counts, abs_counts);

        // 3. Ignore the static-friction deadzone so it doesn't skew the slope
        if (abs_counts > 5) {
            sum_x  += duty;
            sum_y  += abs_counts;
            sum_xy += static_cast<float>(duty) * abs_counts;
            sum_xx += static_cast<float>(duty) * duty;
            valid_samples++;
        }
    }

    // 4. Stop the motor when done
    analogWrite(this->chanPWM, 0);
    ESP_LOGI("motor_cal", "%s: %u/%u usable samples; max encoder count in 100 ms: %d",
             this->nvs_key, static_cast<unsigned>(valid_samples), static_cast<unsigned>(n), max_abs_counts);

    if (valid_samples < 2) {
        this->speed_ratio = 0.0f;
        ESP_LOGW("motor_cal", "%s: calibration failed; check motor power and encoder wiring", this->nvs_key);
        return;
    }

    float denominator = (valid_samples * sum_xx) - (sum_x * sum_x);
    if (denominator == 0.0f) {
        this->speed_ratio = 0.0f;
        ESP_LOGW("motor_cal", "%s: calibration failed; samples had no duty variation", this->nvs_key);
        return;
    }
    float slope = ((valid_samples * sum_xy) - (sum_x * sum_y)) / denominator;
    this->speed_ratio = (slope * 10) / hardware::ENCODER_COUNTS_PER_REVOLUTION;
    if (!std::isfinite(this->speed_ratio) || this->speed_ratio <= 0.0f) {
        this->speed_ratio = 0.0f;
        ESP_LOGW("motor_cal", "%s: calibration produced an invalid ratio", this->nvs_key);
    }
}
void Motor::saveCalibration() {
    if (!std::isfinite(this->speed_ratio) || this->speed_ratio <= 0.0f) return;
    nvs_handle_t handle;
    // Open namespace "motor_cal" in Read/Write mode
    esp_err_t err = nvs_open("motor_cal", NVS_READWRITE, &handle);
    if (err != ESP_OK) return;

    // Safely copy the 4 float bytes into a uint32_t (avoids strict-aliasing bugs)
    uint32_t raw_bits = 0;
    std::memcpy(&raw_bits, &this->speed_ratio, sizeof(float));
    // Store the raw bits in NVS
    nvs_set_u32(handle, this->nvs_key, raw_bits);

    // Always commit changes to write them to flash
    nvs_commit(handle);
    nvs_close(handle);
}
bool Motor::loadCalibration() {
    nvs_handle_t handle;
    // Open namespace in Read-Only mode
    esp_err_t err = nvs_open("motor_cal", NVS_READONLY, &handle);
    if (err != ESP_OK) return false;

    uint32_t raw_bits = 0;
    err = nvs_get_u32(handle, this->nvs_key, &raw_bits);
    nvs_close(handle);

    if (err == ESP_OK) {
        std::memcpy(&this->speed_ratio, &raw_bits, sizeof(float));
        if (std::isfinite(this->speed_ratio) && this->speed_ratio > 0.0f) return true;
        this->speed_ratio = 0.0f;
        ESP_LOGW("motor_cal", "%s: stored ratio is invalid; recalibrating", this->nvs_key);
    }
    return false;
}

void Motor::setDirection(bool forward) {
    this->is_forward = forward;

    gpio_set_level(this->pinA, forward ? 1 : 0);
    gpio_set_level(this->pinB, forward ? 0 : 1);
}
float Motor::speedToDuty(float speed) const {
    if (this->speed_ratio <= 0.0f) return 0.0f;
    float angular_velocity = speed / this->wheel_circumference; // in rev/s
    float duty = angular_velocity / this->speed_ratio; // in duty units
    return std::clamp(duty, -this->max_duty, this->max_duty);
}
void Motor::setTargetSpeed(float speed) {
    // Convert speed (in m/s) to PWM duty cycle
    setDirection(speed >= 0.0f);
    analogWrite(this->chanPWM, static_cast<uint16_t>(std::abs(speedToDuty(speed))));
    this->target_speed = speed;
}
void Motor::setGains(float kp, float ki, float kd) {
    this->kp = kp;
    this->ki = ki;
    this->kd = kd;
}
void Motor::stop() {
    this->target_speed = 0.0f;
    this->integral = 0.0f;
    analogWrite(this->chanPWM, 0);
}


MotorController::MotorController(Motor& motor1, Motor& motor2)
    : motor1(motor1), motor2(motor2) {}

void MotorController::begin(bool force_calibrate) {
    bool motor1_loaded = !force_calibrate && motor1.loadCalibration();
    bool motor2_loaded = !force_calibrate && motor2.loadCalibration();
    if (!motor1_loaded) {
        motor1.calibrate();
        motor1.saveCalibration();
    }
    if (!motor2_loaded) {
        motor2.calibrate();
        motor2.saveCalibration();
    }
}
