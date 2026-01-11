#pragma once
#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_ESP32_SPI

void spi_done_callback(spi_slave_transaction_t *trans);

#endif