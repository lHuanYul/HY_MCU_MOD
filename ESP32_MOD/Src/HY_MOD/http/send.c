#include "HY_MOD/http/send.h"
#ifdef HY_MOD_ESP32_HTTP

static const char *TAG = "MY_HTTP_SEND";

void http_send_handler(const char *url, const char *json_data)
{
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        return;
    }
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_data, strlen(json_data));
    ESP_LOGI(TAG, "Posting to %s...", url);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status = %d, len = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "Failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

#endif