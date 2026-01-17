#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_SPI_JSON

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

#ifdef STM32H7
#ifndef ALIGN_32
    #define ALIGN_32(x)  ((((x) + 31) / 32) * 32)
#endif
    #define SPI_DMA_BUFFER_ATTR __attribute__((section(".RAM_D1"), aligned(32)))
#else
    #define SPI_DMA_BUFFER_ATTR
#endif
/* EXAMPLE
SPI_DMA_BUFFER_ATTR static uint8_t rx_buf[ALIGN_32(JSON_PKT_LEN)];

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