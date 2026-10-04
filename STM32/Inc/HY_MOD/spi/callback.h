#pragma once
#include "HY_MOD/spi/basic.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/packet/json.h"

#define SPI_HAL_TxRxCpltCB_CALL(spi,hspi) \
    do { \
        if ( \
            INSTANCE_CHK((hspi), (spi)->const_h.hspix) \
        ) { \
            spi_tx_rx_cb((spi)); \
        } \
    } while (0)
/*
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_tx_rx_cb(SpiParametar *spi);

#define SPI_HAL_RxCpltCB_CALL(spi,hspi) \
    do { \
        if ( \
            INSTANCE_CHK((hspi), (spi)->const_h.hspix) \
        ) { \
            spi_rx_cb((spi)); \
        } \
    } while (0)
/*
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_rx_cb(SpiParametar *spi);


#define SPI_HAL_TxCpltCB_CALL(spi,hspi) \
    do { \
        if ( \
            INSTANCE_CHK((hspi), (spi)->const_h.hspix) \
        ) { \
            spi_tx_cb((spi)); \
        } \
    } while (0)
/*
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
*/
void spi_tx_cb(SpiParametar *spi);

#endif