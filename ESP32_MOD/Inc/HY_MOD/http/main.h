#pragma once
#include "HY_MOD/http/basic.h"
#ifdef HY_MOD_ESP32_HTTP

esp_err_t http_send(void);
esp_err_t http_start_server(HttpParametar *http);

#endif