#include "wifi.h"

static const EventBits_t WIFI_CONNECTED_BIT = BIT0;
static const EventBits_t WIFI_FAIL_BIT = BIT1;
static EventGroupHandle_t wifi_event_group = nullptr;

void WiFiManager::wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    auto* manager = static_cast<WiFiManager*>(arg);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (manager->retry_count < wifi::MAXIMUM_RETRY) {
            manager->retry_count++;
            esp_wifi_connect();
            ESP_LOGI(wifi::TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const auto* event = static_cast<ip_event_got_ip_t*>(event_data);
        ESP_LOGI(wifi::TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        manager->retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

bool WiFiManager::wifi_init_sta()
{
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, this, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, this, nullptr));

    wifi_config_t wifi_config = {};
    std::memcpy(wifi_config.sta.ssid, ssid,
                strnlen(ssid, sizeof(wifi_config.sta.ssid)));
    std::memcpy(wifi_config.sta.password, password,
                strnlen(password, sizeof(wifi_config.sta.password)));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(tag, "wifi_init_sta finished.");

    const EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(tag, "connected to ap SSID:%s", ssid);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(tag, "failed to connect to SSID:%s", ssid);
    } else {
        ESP_LOGE(tag, "unexpected event");
    }
    return (bits & WIFI_CONNECTED_BIT) != 0;
}


bool WiFiManager::wifi_init()
{
    if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL) {
        /* If you only want to open more logs in the wifi module, you need to make the max level greater than the default level,
         * and call esp_log_level_set() before esp_wifi_init() to improve the log level of the wifi module. */
        esp_log_level_set("wifi", static_cast<esp_log_level_t>(CONFIG_LOG_MAXIMUM_LEVEL));
    }

    ESP_LOGI(wifi::TAG, "ESP_WIFI_MODE_STA");
    return wifi_init_sta();
}
