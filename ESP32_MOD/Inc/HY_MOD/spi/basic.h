#pragma once
#include "main/config.h"
#ifdef HY_MOD_ESP32_SPI

#include "HY_MOD/main/fn_state.h"
#include "driver/gpio.h"
#include "driver/spi_slave.h"

#define SPI_MASTER_ASK "$READ  "
#define SPI_LENGTH_H   "$L:HL  "
#define SPI_SLAVE_EMP  "$NONE  "

typedef struct SpiConst
{
    spi_host_device_t SPIx_HOST;
    int MISO;
    int MOSI;
    int SCK;
    // CS
    int NSS;
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
    spi_slave_transaction_t slave;
    uint16_t rx_buf_len;
    uint8_t *rx_buf;
    uint16_t tx_buf_len;
    uint8_t *tx_buf;
} SpiParametar;


#endif