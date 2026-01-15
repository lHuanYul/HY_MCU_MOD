#pragma once
#include "HY_MOD/connectivity/spi_json/basic.h"
#ifdef HY_MOD_STM32_SPI

Result spi_init(SpiParametar *spi);
Result spi_start_receive(SpiParametar *spi);
Result spi_start_transmit(SpiParametar *spi);

#endif