#include "server.h"
#include "esp_log.h"
#include "MotorController.h"
namespace {
constexpr char TAG[] = "robot_server";
static char last_command = 'S';

esp_err_t root_get_handler(httpd_req_t* request) {
    httpd_resp_set_type(request, "text/plain");
    return httpd_resp_send(request, "Medicine Retrieval Robot is online\n", HTTPD_RESP_USE_STRLEN);
}

esp_err_t status_get_handler(httpd_req_t* req) {
    char response[] = {last_command, '\0'};
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

esp_err_t command_post_handler(httpd_req_t* req) {
    auto* controller = static_cast<MotorController*>(req->user_ctx);
    if (controller == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Motor controller unavailable");
        return ESP_FAIL;
    }
    if (req->content_len != 1) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Send one letter");
        return ESP_OK;
    }

    char command;
    if (httpd_req_recv(req, &command, 1) != 1) {
        return ESP_FAIL;
    }
    if (command == 'F') {
        controller->motor1.setSpeed(100);
        controller->motor2.setSpeed(100);
    } else if (command == 'S') {
        controller->motor1.setSpeed(0);
        controller->motor2.setSpeed(0);
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Use F or S");
        return ESP_OK;
    }
    last_command = command;
    return httpd_resp_send(
        req,
        "saved",
        HTTPD_RESP_USE_STRLEN
    );
}
}

httpd_handle_t start_webserver(MotorController* controller) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;

    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(result));
        return nullptr;
    }

    httpd_uri_t root = {};
    root.uri = "/";
    root.method = HTTP_GET;
    root.handler = root_get_handler;
    result = httpd_register_uri_handler(server, &root);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register root page: %s", esp_err_to_name(result));
        httpd_stop(server);
        return nullptr;
    }
    
    httpd_uri_t status = {};
    status.uri = "/status";
    status.method = HTTP_GET;
    status.handler = status_get_handler;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &status));

    httpd_uri_t command = {};
    command.uri = "/command";
    command.method = HTTP_POST;
    command.handler = command_post_handler;
    command.user_ctx = controller; // Pass the MotorController instance to the handler
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &command));

    ESP_LOGI(TAG, "HTTP server listening on port %d", config.server_port);
    return server;
}
