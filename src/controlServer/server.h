#pragma once

#include "esp_http_server.h"
#include "TaskManager.h"
class MotorController; 

httpd_handle_t start_webserver(TaskManager* task_manager);
