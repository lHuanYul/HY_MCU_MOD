#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

#define SPI_MASTER_ASK "$READ  "
#define SPI_LENGTH_H   "$L:HL  "
#define SPI_SLAVE_EMP  "$NONE  "

typedef struct SpiConst
{
    SPI_HandleTypeDef *hspix;
    GPIOData MISO;
    GPIOData MOSI;
    GPIOData SCK;
    // CS
    GPIOData NSS;
} SpiConst;

typedef enum SpiState
{
    SPI_STATE_FINISH,
    SPI_STATE_ERROR,
    SPI_STATE_RECV_HEADER,
    SPI_STATE_RECV_BODY,
    SPI_STATE_TRSM_HEADER,
    SPI_STATE_TRSM_BODY,
} SpiState;

typedef struct SpiParametar
{
    const SpiConst const_h;
    SpiState state;
    osSemaphoreId_t rx_handle;
    const osSemaphoreAttr_t rx_handle_attr;
    uint16_t rx_buf_len;
    uint8_t *rx_buf;
    osSemaphoreId_t tx_handle;
    const osSemaphoreAttr_t tx_handle_attr;
    uint16_t tx_buf_len;
    uint8_t *tx_buf;
} SpiParametar;

#endif