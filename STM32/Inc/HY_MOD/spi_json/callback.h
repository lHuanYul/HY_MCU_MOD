#pragma once
#include "HY_MOD/spi_json/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/packet/json.h"

void spi_json_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf);
void spi_json_tx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf);

#endif