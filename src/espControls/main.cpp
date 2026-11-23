#include "freertos/FreeRTOS.h" // Changed \ to /
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "MotorController.h"
#include "pins.h"
#include "wifi.h"
#include "server.h"
extern "C" void app_main() {
    init_motor_gpio();
    Motor motor1 = {
                    .speed = 0, 
                    .pinA = pinNums::AIN1,
                    .pinB = pinNums::AIN2,
                    .pinPWM = pinNums::PWMA,
                    .encoderA = pinNums::ENCODER_1.A,
                    .encoderB = pinNums::ENCODER_1.B
                };
    Motor motor2 = {
                    .speed = 0,
                    .pinA = pinNums::BIN1,
                    .pinB = pinNums::BIN2,
                    .pinPWM = pinNums::PWMB,
                    .encoderA = pinNums::ENCODER_2.A,
                    .encoderB = pinNums::ENCODER_2.B
                };
    static MotorController controller(motor1, motor2);
    pcnt_unit_handle_t encoder1 = configure_encoder(pinNums::ENCODER_1);
    pcnt_unit_handle_t encoder2 = configure_encoder(pinNums::ENCODER_2);

    static WiFiManager wifiManager(wifi::ssid, wifi::password, wifi::TAG);
    gpio_set_level(pinNums::STBY, 1);
    gpio_set_level(motor1.pinPWM, 1);
    gpio_set_level(motor2.pinPWM, 1);
    if (wifiManager.wifi_init()) {
        start_webserver(&controller);
    }
}
