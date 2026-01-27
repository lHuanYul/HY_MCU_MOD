#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_SPI_JSON

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"
#include "HY_MOD/main/variable_cal.h"
#include "HY_MOD/packet/json.h"
#include "HY_MOD/spi/basic.h"

#define SPI_JSON_STATE_OK     0
#define SPI_JSON_STATE_HEAD   2
#define SPI_JSON_STATE_BODY   4
#define SPI_JSON_STATE_ERR    15

#define SPI_JSON_STATE_GET(spi) FLAGS_GET(spi->state, BIT_0_3_MASK)
#define SPI_JSON_STATE_SET(spi, val) FLAGS_SET(spi->state, BIT_0_3_MASK, val)

#define SPI_LENGTH_H   "$L:HL  "

/* EXAMPLE
// SPI_DMA_BUFFER_ATTR static uint8_t rx_buf[ALIGN_32(JSON_PKT_LEN)];

STM32H7 REMBER TO SET MPU (SET IN CUBEMX)
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
*/

typedef struct SpiJsonParametar
{
    SpiParametar spi_p;
    uint8_t     state;
    JsonPkt     *rx_pkt;
    JsonPkt     *tx_pkt;
    JsonPkt     *tx_hold;
} SpiJsonParametar;

Result spi_json_init(SpiJsonParametar *spi);

#endif