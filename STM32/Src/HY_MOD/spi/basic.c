#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/main/variable_cal.h"
#include "spi.h"

// static ATTR_RAM_D2_ALIGN_32 uint8_t spi_trash[SPI_DATA_MAX];

SpiParametar spi1_h = {
    .const_h =
    {
        .hspix = &hspi1,
        // .dma_tx = &hdma_spi1_tx,
        // .dma_rx = &hdma_spi1_rx,
        // .SCK  = { GPIOA, GPIO_PIN_5 },
        // .MISO = { GPIOA, GPIO_PIN_6 },
        // .MOSI = { GPIOB, GPIO_PIN_5 },
    },
    .txrx.attr = {
        .name = "Spi1TxRxSem"
    },
    .rx.attr = {
        .name = "Spi1RxSem"
    },
    .tx.attr = {
        .name = "Spi1TxSem"
    },
};

SpiParametar spi3_h = {
    .const_h =
    {
        .hspix = &hspi3,
        // .SCK  = { GPIOB, GPIO_PIN_3 },
        // .MOSI = { GPIOD, GPIO_PIN_6 },
    },
    .txrx.attr = {
        .name = "Spi3TxRxSem"
    },
    .rx.attr = {
        .name = "Spi3RxSem"
    },
    .tx.attr = {
        .name = "Spi3TxSem"
    },
};

void spi_cs_select(const SpiCSConst *cs, uint32_t tick)
{
    if (cs->high_sel)
        GPIO_WRITE(cs->CS_NSS, 1);
    else
        GPIO_WRITE(cs->CS_NSS, 0);
    for (volatile uint32_t i = 0; i < tick; i++) __NOP();
}

void spi_cs_unselect(const SpiCSConst *cs, uint32_t tick)
{
    if (cs->high_sel)
        GPIO_WRITE(cs->CS_NSS, 0);
    else
        GPIO_WRITE(cs->CS_NSS, 1);
    for (volatile uint32_t i = 0; i < tick; i++) __NOP();
}

Result spi_init(SpiParametar *spi)
{
    spi->txrx.id    = osSemaphoreNew(1, 0, &spi->txrx.attr);
    spi->rx.id      = osSemaphoreNew(1, 0, &spi->rx.attr);
    spi->tx.id      = osSemaphoreNew(1, 0, &spi->tx.attr);
    return RESULT_OK(spi);
}

static Result wait_finish(
    SpiParametar *spi, osSemaphoreId_t tag, uint32_t time_out
) {
    if (osSemaphoreAcquire(tag, time_out) != osOK)
    {
        HAL_SPI_Abort(spi->const_h.hspix);
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    }
#if defined(STM32G0) || defined(STM32G4)
    while (__HAL_SPI_GET_FLAG(spi->const_h.hspix, SPI_FLAG_BSY));
#elif defined(STM32H7)
    for (volatile int i = 0; i < 20; i++) __NOP();
#endif
    return RESULT_OK(spi);
}

Result spi_start_transmit_it(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
) {
    if (HAL_SPI_Transmit_IT(spi->const_h.hspix, buf, len) != HAL_OK)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    return wait_finish(spi, spi->tx.id, 100);
}

Result spi_start_transmit_dma(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
) {
    // HAL_SPI_Transmit_IT HAL_SPI_Transmit_DMA
#ifdef STM32H7
    uint32_t addr = (uint32_t)buf & ~0x1FUL;
    int32_t size = ((int32_t)len + ((uint32_t)buf & 0x1FUL) + 31) & ~0x1FUL;
    SCB_CleanDCache_by_Addr((uint32_t*)addr, size);
#endif
    if (HAL_SPI_Transmit_DMA(spi->const_h.hspix, buf, len) != HAL_OK)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    return wait_finish(spi, spi->tx.id, 100);
}

Result spi_start_receive_dma(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
) {
    // HAL_SPI_Receive_IT HAL_SPI_Receive_DMA
    if (HAL_SPI_Receive_DMA(spi->const_h.hspix, buf, len) != HAL_OK)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    Result res = wait_finish(spi, spi->rx.id, 100);
#ifdef STM32H7
    if (RESULT_CHECK_OK(res))
        SCB_InvalidateDCache_by_Addr((uint32_t*)buf, len);
#endif
    return res;
}

Result spi_start_transceive_dma(
    SpiParametar *spi,
    uint8_t *tx_buf, uint8_t *rx_buf,
    uint16_t len
) {
#ifdef STM32H7
    SCB_CleanDCache_by_Addr((uint32_t*)tx_buf, len);
#endif
    if (HAL_SPI_TransmitReceive_DMA(spi->const_h.hspix, tx_buf, rx_buf, len) != HAL_OK)
        return RESULT_ERROR(RESULT_ERROR_FAIL);
    Result res = wait_finish(spi, spi->txrx.id, 100);
#ifdef STM32H7
    if (RESULT_CHECK_OK(res))
        SCB_InvalidateDCache_by_Addr((uint32_t*)rx_buf, len);
#endif
    return res;
}

#endif