#pragma once
#include <cstring>
#include "secrets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

namespace wifi {
inline constexpr const char* ssid = WIFI_SSID;
inline constexpr const char* password = WIFI_PASSWORD;
inline constexpr const char* TAG = "my_wifi";
inline constexpr int MAXIMUM_RETRY = 10;
};

class WiFiManager {
    private:
        static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
        bool wifi_init_sta();
    public:
        const char* ssid;
        const char* password;
        const char* tag;
        int retry_count = 0;
        WiFiManager(const char* ssid, const char* password, const char* tag)
            : ssid(ssid), password(password), tag(tag) {}
        bool wifi_init();
};

