#pragma once
#include "main/config.h"
#ifdef HY_MOD_ESP32_SPI

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/packet/json.h"
#include "driver/gpio.h"
#include "driver/spi_slave.h"

#define SPI_JSON_STATE_OK     0
#define SPI_JSON_STATE_HEAD   2
#define SPI_JSON_STATE_BODY   4
#define SPI_JSON_STATE_ERR    15

#define SPI_JSON_STATE_GET(spi) FLAGS_GET(spi->state, BIT_0_3_MASK)
#define SPI_JSON_STATE_SET(spi, val) FLAGS_SET(spi->state, BIT_0_3_MASK, val)

#define SPI_LENGTH_H   "$L:HL  "

#define U8_BIT_SIZE 8

typedef struct SpiConst
{
    spi_host_device_t SPIx_HOST;
    int MISO;
    int MOSI;
    int SCK;
    // CS
    int NSS;
} SpiConst;

typedef struct SpiParametar
{
    const SpiConst const_h;
    spi_slave_transaction_t slave;
    TaskHandle_t txrx;
} SpiParametar;

typedef struct SpiJsonParametar
{
    SpiParametar spi_p;
    uint8_t     state;
    JsonPkt     *rx_pkt;
    JsonPkt     *tx_pkt;
    JsonPkt     *tx_hold;
} SpiJsonParametar;

#endif