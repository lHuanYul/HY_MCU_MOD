#pragma once
#include "HY_MOD/http/basic.h"
#ifdef HY_MOD_ESP32_HTTP

extern MyHttpBuffer http_recv_ctx;

esp_err_t http_recv_handler(httpd_req_t *req);

#endif