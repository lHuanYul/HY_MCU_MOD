#pragma once
#include "HY_MOD/spi_json/basic.h"
#ifdef HY_MOD_STM32_SPI_JSON

Result spi_init(SpiJsonParametar *spi);
Result spi_start_receive(SpiJsonParametar *spi);
Result spi_start_transmit(SpiJsonParametar *spi);

#endif