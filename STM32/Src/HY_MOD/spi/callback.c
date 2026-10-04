#include "HY_MOD/spi/callback.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"

void spi_tx_rx_cb(SpiParametar *spi)
{
    osSemaphoreRelease(spi->txrx.id);
}

void spi_rx_cb(SpiParametar *spi)
{
    osSemaphoreRelease(spi->rx.id);
}

void spi_tx_cb(SpiParametar *spi)
{
    osSemaphoreRelease(spi->tx.id);
}

#endif