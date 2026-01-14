#pragma once
#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_ESP32_SPI

void spi_init(SpiParametar *spi);
void spi_recv_start(SpiParametar *spi, uint16_t byte_len, uint8_t *rx_buf, uint32_t timeout_ms);
void spi_trsm_start(SpiParametar *spi, uint16_t byte_len, uint8_t *tx_buf, uint32_t timeout_ms);

#endif