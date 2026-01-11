#include "HY_MOD/http/receive.h"
#ifdef HY_MOD_ESP32_HTTP

static const char *TAG = "MY_HTTP_RECV";

/*
// !
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    MyHttpBuffer *ctx = (MyHttpBuffer *)evt->user_data;
    switch (evt->event_id)
    {
        case HTTP_EVENT_ERROR:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        }
        case HTTP_EVENT_ON_CONNECTED:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        }
        case HTTP_EVENT_HEADER_SENT:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        }
        case HTTP_EVENT_ON_HEADER:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        }
        case HTTP_EVENT_ON_DATA:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (!ctx || !ctx->buffer)
            {
                ESP_LOGE(TAG, "BUFFER unset");
            }
            ctx->len = 0;
            // we are just starting to copy the output data into the use
            memset(ctx->buffer, 0, ctx->max_len);
            
            // Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
            // However, event handler can also be used in case chunked encoding is used.
            if (!esp_http_client_is_chunked_response(evt->client))
            {
                // If user_data buffer is configured, copy the response into the buffer
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                uint32_t copy_len = MIN(ctx->buffer, (ctx->max_len - ctx->len));
                if (copy_len)
                {
                    memcpy(ctx->buffer + ctx->len, evt->data, copy_len);
                    ctx->len += copy_len;
                }
            }
            else
            {
                ESP_LOGE(TAG, "chunked_response err");
            }
            break;
        }
        case HTTP_EVENT_ON_FINISH:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        }
        case HTTP_EVENT_DISCONNECTED:
        {
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            uint32_t mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0)
            {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            break;
        }
        case HTTP_EVENT_REDIRECT:
        {
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
        }
    }
    return ESP_OK;
}
*/

__attribute__((weak)) void http_recv_proc(HttpParametar *http) { return; };
esp_err_t http_recv_handler(httpd_req_t *req)
{
    HttpParametar *http = (HttpParametar *)req->user_ctx;
    MyHttpBuffer *rx_buf = &http->rx_buf;
    if (req->content_len > rx_buf->max_len)
    {
        /* Respond with 500 Internal Server Error */
        ESP_LOGE(TAG, "content too long");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    rx_buf->len = req->content_len;

    uint32_t received = 0;
    uint32_t cur_len = 0;
    while (cur_len < rx_buf->len)
    {
        received = httpd_req_recv(req, rx_buf->buffer + cur_len, rx_buf->len - cur_len);
        if (received <= 0)
        {
            /* Respond with 500 Internal Server Error */
            ESP_LOGE(TAG, "recv Failed");
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "recv Failed");
            return ESP_FAIL;
        }
        cur_len += received;
    }

    httpd_resp_sendstr(req, "Post control value successfully");
    http_recv_proc(http);
    return ESP_OK;
}

#endif