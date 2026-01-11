#pragma once
#include "main/config.h"
#ifdef HY_MOD_ESP32_HTTP
#include "esp_http_server.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

typedef struct MyHttpBuffer {
    char buffer[JSON_PKT_LEN];
    uint32_t len;
    uint32_t max_len;
} MyHttpBuffer;

typedef struct HttpConst
{
    bool e;
} HttpConst;

typedef struct HttpParametar
{
    const HttpConst const_h;
    httpd_handle_t server;
    httpd_config_t config;
    MyHttpBuffer rx_buf;
    httpd_uri_t rx_recv;
    MyHttpBuffer tx_buf;
} HttpParametar;

#endif