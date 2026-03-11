#include "HY_MOD/http/task.h"
#ifdef HY_MOD_ESP32_HTTP

#include "main/mod_cfg.h"
#include "HY_MOD/wifi/main.h"
#include "HY_MOD/http/main.h"

void StartHttpTask(void *argument)
{
    my_wifi_connect();
    vTaskDelay(pdMS_TO_TICKS(1000));
    HttpParametar *http = &http_h;
    RESULT_CHECK_HANDLE(http_start_server(http));
    while (1)
    {
        // http_send();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    vTaskDelete(NULL);
}

#endif