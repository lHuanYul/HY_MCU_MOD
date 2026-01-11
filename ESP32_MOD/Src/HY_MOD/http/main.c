#include "HY_MOD/http/main.h"
#ifdef HY_MOD_ESP32_HTTP

#include "HY_MOD/http/send.h"
#include "HY_MOD/http/receive.h"

static const char *TAG = "MY_HTTP_MAIN";

/*
// !
#define CONFIG_EXAMPLE_HTTP_ENDPOINT "httpbin.org"
#define MAX_HTTP_OUTPUT_BUFFER 2048

void http_rest_with_url(void)
{
    // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
    // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};

    // NOTE: All the configuration parameters for http_client must be specified either in URL or as host and path parameters.
    // If host and path parameters are not set, query parameter will be ignored. In such cases,
    // query parameter should be specified in URL.
    // If URL as well as host and path parameters are specified, values of host and path will be considered.
    
    esp_http_client_config_t config = {
        .host = CONFIG_EXAMPLE_HTTP_ENDPOINT,
        .path = "/get",
        .query = "esp",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // GET
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }
    ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));

    // POST
    const char *post_data = "{\"field1\":\"value1\"}";
    esp_http_client_set_url(client, "http://"CONFIG_EXAMPLE_HTTP_ENDPOINT"/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
*/

esp_err_t http_send(void)
{
    const char *target_url = "http://192.168.0.21/set-temperature";
    const char *my_json = "{\"type\": \"Normal\", \"value\": 27.0}";
    http_send_handler(target_url, my_json);
    return ESP_OK;
}

#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

__attribute__((weak)) void http_recv_register(HttpParametar *http) { return; };
esp_err_t http_start_server(HttpParametar *http)
{
    http->config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "HTTP Server Starting");
    if (httpd_start(&http->server, &http->config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Start server failed");
        return ESP_FAIL;
    }
    http_recv_register(http);

    ESP_LOGI(TAG, "HTTP Server Started");
    return ESP_OK;
}

#endif