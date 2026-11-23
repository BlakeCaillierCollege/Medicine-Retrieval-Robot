#pragma once

#include "esp_http_server.h"

class MotorController; 

httpd_handle_t start_webserver(MotorController* controller);
