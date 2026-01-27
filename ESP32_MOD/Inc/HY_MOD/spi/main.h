#pragma once
#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_ESP32_SPI

void spi_init(SpiParametar *spi);
Result spi_start_transceive(SpiParametar *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len, uint32_t timeout_ms);
Result spi_start_receive(SpiParametar *spi, uint16_t byte_len, uint8_t *rx_buf, uint32_t timeout_ms);
Result spi_start_transmit(SpiParametar *spi, uint16_t byte_len, uint8_t *tx_buf, uint32_t timeout_ms);

#endif