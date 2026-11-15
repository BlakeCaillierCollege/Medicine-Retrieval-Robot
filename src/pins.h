#include "driver/gpio.h"
#include "driver/pulse_cnt.h"

namespace pinNumbers {
    constexpr int PWMA = 12;
    constexpr int AIN2 = 14;
    constexpr int AIN1 = 27;
    constexpr int STBY = 26;
    constexpr int BIN1 = 25;
    constexpr int BIN2 = 33;
    constexpr int PWMB = 32;
    constexpr int ENCODER_A = 3;
    constexpr int ENCODER_B = 35;
}

gpio_config_t motor_input_config = {
    .pin_bit_mask = 1ULL << pinNumbers::AIN1 | 1ULL << pinNumbers::AIN2 | 1ULL << pinNumbers::STBY | 1ULL << pinNumbers::BIN1 | 1ULL << pinNumbers::BIN2,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};
