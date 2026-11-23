#pragma once
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "driver/ledc.h"
#include "esp_err.h"

struct encoder_pins_t {
    gpio_num_t A;
    gpio_num_t B;
};

namespace pinNums {
    constexpr gpio_num_t AIN1 = GPIO_NUM_27;
    constexpr gpio_num_t AIN2 = GPIO_NUM_14;
    constexpr gpio_num_t STBY = GPIO_NUM_26;
    constexpr gpio_num_t BIN1 = GPIO_NUM_25;
    constexpr gpio_num_t BIN2 = GPIO_NUM_33;
    constexpr gpio_num_t PWMA = GPIO_NUM_12;
    constexpr gpio_num_t PWMB = GPIO_NUM_32;

    constexpr encoder_pins_t ENCODER_1 = {GPIO_NUM_19, GPIO_NUM_18};
    constexpr encoder_pins_t ENCODER_2 = {GPIO_NUM_17, GPIO_NUM_5};
}

namespace pwmconfigs {
    constexpr ledc_mode_t MODE = LEDC_LOW_SPEED_MODE;
    constexpr ledc_timer_bit_t  DUTY_RES = LEDC_TIMER_11_BIT;
    constexpr ledc_timer_t TIMER = LEDC_TIMER_0;
    constexpr uint32_t FREQUENCY = 4000;
    constexpr soc_periph_ledc_clk_src_legacy_t CLK_SRC = LEDC_AUTO_CLK;
    constexpr gpio_num_t OUTPUT_IO = GPIO_NUM_4;
    
    constexpr ledc_channel_t chanA = LEDC_CHANNEL_0;
    constexpr ledc_channel_t chanB = LEDC_CHANNEL_1;
}

void GPIOinit();
void PWMinit();
void analogWrite(ledc_channel_t channel, uint8_t duty);
pcnt_unit_handle_t configure_encoder(encoder_pins_t pins, ledc_channel_t channel);