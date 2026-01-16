#include "HY_MOD/spi_json/main.h"
#ifdef HY_MOD_STM32_SPI
#include "HY_MOD/main/variable_cal.h"
#include "spi.h"

Result spi_init(SpiParametar *spi)
{
    spi->rx_handle = osSemaphoreNew(1, 0, &spi->rx_handle_attr);
    spi->tx_handle = osSemaphoreNew(1, 0, &spi->tx_handle_attr);
    return RESULT_OK(NULL);
}

static Result wait_it(SpiParametar *spi, osSemaphoreId_t tag, uint32_t time_out)
{
    if (osSemaphoreAcquire(tag, time_out) != osOK)
    {
        HAL_SPI_DMAStop(spi->const_h.hspix);
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    return RESULT_OK(NULL);
}

Result spi_start_receive(SpiParametar *spi)
{
    GPIO_WRITE(spi->const_h.NSS, 0);
    for(volatile int i = 0; i < 800; i++) __NOP();
    // HAL_SPI_Receive_IT HAL_SPI_Receive_DMA
    if (HAL_SPI_Receive_DMA(spi->const_h.hspix, spi->rx_buf, spi->rx_buf_len) != HAL_OK)
    {
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    wait_it(spi, spi->rx_handle, 100);
    return RESULT_OK(NULL);
}

Result spi_start_transmit(SpiParametar *spi)
{
    GPIO_WRITE(spi->const_h.NSS, 0);
    for(volatile int i = 0; i < 800; i++) __NOP();
    // HAL_SPI_Transmit_IT HAL_SPI_Transmit_DMA
#ifdef STM32H7
    if (HAL_SPI_TransmitReceive_DMA(spi->const_h.hspix, spi->tx_buf, spi->rx_buf, spi->tx_buf_len) != HAL_OK)
#else
    if (HAL_SPI_Transmit_DMA(spi->const_h.hspix, spi->tx_buf, spi->tx_buf_len) != HAL_OK)
#endif
    {
        GPIO_WRITE(spi->const_h.NSS, 1);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    wait_it(spi, spi->tx_handle, 100);
#ifdef STM32G0
    while (__HAL_SPI_GET_FLAG(spi->const_h.hspix, SPI_FLAG_BSY));
#endif
    return RESULT_OK(NULL);
}

#endif