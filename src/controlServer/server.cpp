#include "server.h"
#include "esp_log.h"
#include "MotorController.h"
#include "page_assets.h"
namespace {
    constexpr char TAG[] = "robot_server";
    static char last_command = 'S';
    // ============================================

    //  Web server handlers

    // ============================================
    esp_err_t root_get_handler(httpd_req_t* request) {
        httpd_resp_set_type(request, "text/html");
        return httpd_resp_send(request, page_html, HTTPD_RESP_USE_STRLEN);
    }

    esp_err_t page_js_get_handler(httpd_req_t* request) {
        httpd_resp_set_type(request, "application/javascript");
        return httpd_resp_send(request, page_js, HTTPD_RESP_USE_STRLEN);
    }

    esp_err_t page_css_get_handler(httpd_req_t* request) {
        httpd_resp_set_type(request, "text/css");
        return httpd_resp_send(request, page_css, HTTPD_RESP_USE_STRLEN);
    }
    // ============================================

    // Telemetry handlers

    // ============================================
    esp_err_t PID_get_handler(httpd_req_t* req) {
        auto* task_mgr = static_cast<TaskManager*>(req->user_ctx);
        if (task_mgr == nullptr) {
            return ESP_FAIL;
        }
        httpd_resp_set_type(req, "application/octet-stream");
        PIDSample samples[16];
        size_t count;
        do {
            count = task_mgr->drainSamples(samples, 16);
            if (count > 0) {
                esp_err_t result = httpd_resp_send_chunk(
                    req, reinterpret_cast<const char*>(samples), count * sizeof(PIDSample));
                if (result != ESP_OK) return result;
            }
        } while (count == 16);
        return httpd_resp_send_chunk(req, nullptr, 0);
    }
    // =============================================

    // Robot status handlers

    // =============================================
    esp_err_t status_get_handler(httpd_req_t* req) {
        const char status[] = {last_command, '\0'};
        return httpd_resp_send(req, status, HTTPD_RESP_USE_STRLEN);
    }
    // =============================================

    // Command Handlers

    // =============================================
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
            controller->motor1.setTargetSpeed(0.25f);
            controller->motor2.setTargetSpeed(0.25f);
        } else if (command == 'S') {
            controller->motor1.stop();
            controller->motor2.stop();
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

httpd_handle_t start_webserver(TaskManager* task_mgr) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;
    // boots up the server with the default configuration
    esp_err_t result = httpd_start(&server, &config);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(result));
        return nullptr;
    }
    // adds the root handler
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

    // this is the handler for the functions of the buttons
    httpd_uri_t page_js = {};
    page_js.uri = "/page.js";
    page_js.method = HTTP_GET;
    page_js.handler = page_js_get_handler;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &page_js));

    // this is the handler for the look of the page (other than text)
    httpd_uri_t page_css = {};
    page_css.uri = "/page.css";
    page_css.method = HTTP_GET;
    page_css.handler = page_css_get_handler;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &page_css));

    httpd_uri_t status = {};
    status.uri = "/status";
    status.method = HTTP_GET;
    status.handler = status_get_handler;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &status));

    httpd_uri_t PID_status = {};
    PID_status.uri = "/get/PID";
    PID_status.method = HTTP_GET;
    PID_status.handler = PID_get_handler;
    PID_status.user_ctx = task_mgr;
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &PID_status));

    httpd_uri_t command = {};
    command.uri = "/command";
    command.method = HTTP_POST;
    command.handler = command_post_handler;
    command.user_ctx = &task_mgr->getController(); // Pass the MotorController instance to the handler
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &command));

    ESP_LOGI(TAG, "HTTP server listening on port %d", config.server_port);
    return server;
}