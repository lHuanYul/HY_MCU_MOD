#pragma once
#include "HY_MOD/http/basic.h"
#ifdef HY_MOD_ESP32_HTTP

#include "HY_MOD/main/fn_state.h"

Result http_send(HttpParametar *http);
Result http_start_server(HttpParametar *http);

#endif