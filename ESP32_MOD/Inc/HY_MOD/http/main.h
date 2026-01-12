#pragma once
#include "HY_MOD/http/basic.h"
#ifdef HY_MOD_ESP32_HTTP

#include "HY_MOD/main/fn_state.h"

Result http_send(void);
Result http_start_server(HttpParametar *http);

#endif