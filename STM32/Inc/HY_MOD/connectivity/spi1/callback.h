#pragma once
#include "HY_MOD/connectivity/spi1/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/packet/json.h"

void spi1_rx_callback(SpiParametar *spi, JsonPktPool *pool, JsonPktBuf *buf);
void spi1_tx_callback(SpiParametar *spi, JsonPktPool *pool, JsonPktBuf *buf);

#endif