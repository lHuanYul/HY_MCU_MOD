#pragma once
#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/packet/json.h"

void spi_tx_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi);
void spi_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi);
void spi_tx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi);

#endif