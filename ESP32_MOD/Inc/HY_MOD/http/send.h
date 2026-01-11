#pragma once
#include "HY_MOD/http/basic.h"
#ifdef HY_MOD_ESP32_HTTP

#include "esp_http_client.h"

void http_send_handler(const char *url, const char *json_data);

#endif