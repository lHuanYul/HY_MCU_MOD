#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_SPI_JSON)
#define HY_MOD_STM32_SPI

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"
#include "HY_MOD/main/free_rtos.h"

typedef struct SpiConst
{
    SPI_HandleTypeDef *hspix;
    GPIOData MISO;
    GPIOData MOSI;
    GPIOData SCK;
    // CS
    GPIOData NSS;
} SpiConst;

// #ifdef STM32H7
// #ifndef ALIGN_32
//     #define ALIGN_32(x)  ((((x) + 31) / 32) * 32)
// #endif
//     #define SPI_DMA_BUFFER_ATTR __attribute__((section(".RAM_D1"), aligned(32)))
// #else
//     #define SPI_DMA_BUFFER_ATTR
// #endif
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
    OsSmpParametar txrx;
    OsSmpParametar rx;
    OsSmpParametar tx;
} SpiParametar;

Result spi_init(SpiParametar *spi);
Result spi_start_receive(SpiParametar *spi, uint8_t *buf, uint16_t len);
Result spi_start_transmit(SpiParametar *spi, uint8_t *buf, uint16_t len);
Result spi_start_transceive(SpiParametar *spi, uint8_t *tx_buf, uint8_t *rx_buf, uint16_t len);

#endif