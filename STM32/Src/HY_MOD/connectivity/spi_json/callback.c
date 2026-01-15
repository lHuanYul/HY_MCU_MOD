#include "HY_MOD/connectivity/spi_json/callback.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"

/*
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_json_rx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf)
{
    if (hspi != spi->const_h.hspix) return;
    GPIO_WRITE(spi->const_h.NSS, 1);
    #if defined(STM32F7) || defined(STM32H7)
        SCB_InvalidateDCache_by_Addr((uint32_t*)spi->rx_buf, JSON_PKT_LEN+32);
    #endif
    
    switch (spi->state)
    {
        case SPI_STATE_RECV_HEADER:
        case SPI_STATE_RECV_BODY:
        {
            break;
        }
        default: spi->state = SPI_STATE_ERROR;
    }
    osSemaphoreRelease(spi->rx_handle);
}

/*
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_json_tx_cb(SpiParametar *spi, SPI_HandleTypeDef *hspi, JsonPktPool *pool, JsonPktBuf *buf)
{
    if (hspi != spi->const_h.hspix) return;
    switch (spi->state)
    {
        case SPI_STATE_FINISH:
        case SPI_STATE_TRSM_HEADER:
        case SPI_STATE_TRSM_BODY:
        {
            break;
        }
        default: spi->state = SPI_STATE_ERROR;
    }
    osSemaphoreRelease(spi->tx_handle);
}

#endif