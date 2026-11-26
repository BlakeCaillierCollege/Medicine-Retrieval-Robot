#include "pins.h"


void GPIOinit() {
    gpio_config_t motor_input_config = {
    .pin_bit_mask = 1ULL << hardware::AIN1 | 1ULL << hardware::AIN2 | 1ULL << hardware::STBY |
                    1ULL << hardware::BIN1 | 1ULL << hardware::BIN2,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&motor_input_config));
    ESP_ERROR_CHECK(gpio_set_level(hardware::STBY, 1));
}

void PWMinit(ledc_channel_t channel, gpio_num_t pin) {
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t pwm_timer = {};
    pwm_timer.speed_mode = pwmconfigs::MODE;
    pwm_timer.duty_resolution = pwmconfigs::DUTY_RES;
    pwm_timer.timer_num = pwmconfigs::TIMER;
    pwm_timer.freq_hz = pwmconfigs::FREQUENCY;
    pwm_timer.clk_cfg = pwmconfigs::CLK_SRC;
    ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t pwm_channel = {};
    pwm_channel.gpio_num = pin;
    pwm_channel.speed_mode = pwmconfigs::MODE;
    pwm_channel.channel = channel;
    pwm_channel.timer_sel = pwmconfigs::TIMER;
    ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));
}

pcnt_unit_handle_t configure_encoder(encoder_pins_t pins) {
    pcnt_unit_config_t unit_config = {};
    unit_config.low_limit = -32767;
    unit_config.high_limit = 32767;

    pcnt_unit_handle_t unit = nullptr;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &unit));

    pcnt_glitch_filter_config_t filter_config = {};
    filter_config.max_glitch_ns = 1000;
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(unit, &filter_config));

    pcnt_chan_config_t channel_a_config = {};
    channel_a_config.edge_gpio_num = pins.A;
    channel_a_config.level_gpio_num = pins.B;
    pcnt_channel_handle_t channel_a = nullptr;
    ESP_ERROR_CHECK(pcnt_new_channel(unit, &channel_a_config, &channel_a));

    pcnt_chan_config_t channel_b_config = {};
    channel_b_config.edge_gpio_num = pins.B;
    channel_b_config.level_gpio_num = pins.A;
    pcnt_channel_handle_t channel_b = nullptr;
    ESP_ERROR_CHECK(pcnt_new_channel(unit, &channel_b_config, &channel_b));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        channel_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        channel_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        channel_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        channel_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(unit));
    ESP_ERROR_CHECK(pcnt_unit_start(unit));
    return unit;
}


void analogWrite(ledc_channel_t channel, uint16_t duty) {
    if (duty > 2047) {
        duty = 2047;
    }
    ESP_ERROR_CHECK(ledc_set_duty(pwmconfigs::MODE, channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(pwmconfigs::MODE, channel));
}

void initPins() {
    GPIOinit();
    PWMinit(hardware::chanA, hardware::PWMA);
    PWMinit(hardware::chanB, hardware::PWMB);
}
