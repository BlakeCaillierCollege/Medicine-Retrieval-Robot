#include "MotorController.h"
#include "pins.h"
#include "wifi.h"
#include "server.h"
#include "esp_log.h"
#include "TaskManager.h"
void initNVS();

extern "C" void app_main() {
    // hardware initialization
    initPins();
    initNVS();
    pcnt_unit_handle_t encoder1 = configure_encoder(hardware::ENCODER_A);
    pcnt_unit_handle_t encoder2 = configure_encoder(hardware::ENCODER_B);
    static Motor motor1(hardware::AIN1, hardware::AIN2, hardware::chanA, encoder1, "motor1");
    static Motor motor2(hardware::BIN1, hardware::BIN2, hardware::chanB, encoder2, "motor2");
    static MotorController controller(motor1, motor2);
    controller.begin(false); // false means don't force calibration on startup
    controller.motor1.setGains(0.5f, 0.1f, 0.01f);
    controller.motor2.setGains(0.5f, 0.1f, 0.01f);
    static TaskManager task_manager(controller);
    if (task_manager.begin() != ESP_OK) {
        // TODO: Report task startup failure through a device status indicator.
        return;
    }
    // WiFi initialization and web server start
    static WiFiManager wifiManager(wifi::ssid, wifi::password, wifi::TAG);
    if (wifiManager.wifi_init()) {
        start_webserver(&task_manager);
    }
}

void initNVS() {
    esp_err_t err = nvs_flash_init();
    // If the NVS partition was truncated or updated, erase and re-init
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}
