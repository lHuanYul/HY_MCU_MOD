#pragma once
#include "HY_MOD/connectivity/spi1/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/packet/json.h"

void spi_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf);
void spi_tx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf);

#endif