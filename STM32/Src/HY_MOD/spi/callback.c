#include "HY_MOD/spi/callback.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"

/*
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_tx_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi)
{
    if (hspi != spi->const_h.hspix) return;
    GPIO_WRITE(spi->const_h.NSS, 1);
    osSemaphoreRelease(spi->txrx.id);
}

/*
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi)
{
    if (hspi != spi->const_h.hspix) return;
    GPIO_WRITE(spi->const_h.NSS, 1);
    osSemaphoreRelease(spi->rx.id);
}

/*
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_tx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi)
{
    if (hspi != spi->const_h.hspix) return;
    GPIO_WRITE(spi->const_h.NSS, 1);
    osSemaphoreRelease(spi->tx.id);
}

#endif