#pragma once
#include "main/config.h"
#ifdef HY_MOD_ESP32_HTTP
#include "esp_http_server.h"
#include "esp_http_client.h"

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
    httpd_config_t config_server;
    MyHttpBuffer rx_buf;
    httpd_uri_t rx_recv;
    esp_http_client_config_t config_client;
    MyHttpBuffer tx_buf;
} HttpParametar;

#endif