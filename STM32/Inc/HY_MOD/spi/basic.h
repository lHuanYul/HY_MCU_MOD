#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_SPI

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"
#include "HY_MOD/main/free_rtos.h"
#include "HY_MOD/main/buffer.h"

/* EXAMPLE
ATTR_RAM_D1_ALIGN_32 static uint8_t rx_buf[ALIGN_32(JSON_PKT_LEN)];

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

typedef struct SpiConst
{
    SPI_HandleTypeDef *hspix;
    // DMA_HandleTypeDef *dma_tx;
    // DMA_HandleTypeDef *dma_rx;
} SpiConst;

typedef struct SpiParametar
{
    const SpiConst const_h;
    // const GPIOData MISO;
    // const GPIOData MOSI;
    // const GPIOData SCK;
    OsSmpParametar txrx;
    OsSmpParametar rx;
    OsSmpParametar tx;
} SpiParametar;

extern SPI_HandleTypeDef hspi1 __weak;
extern SpiParametar spi1_h;
extern SPI_HandleTypeDef hspi3 __weak;
extern SpiParametar spi3_h;

typedef struct SpiCSConst
{
    bool            high_sel;
    GPIOData        CS_NSS;
    uint16_t        delay;
} SpiCSConst;

/**
 * @brief 致能片選 (Select Chip) 並執行建立延遲
 */
void spi_cs_select(const SpiCSConst *cs, uint32_t tick);
/**
 * @brief 釋放片選 (Unselect Chip) 並執行保持延遲
 */
void spi_cs_unselect(const SpiCSConst *cs, uint32_t tick);

Result spi_init(SpiParametar *spi);
Result spi_start_transmit_it(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
);
Result spi_start_transmit_dma(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
);
Result spi_start_receive_dma(
    SpiParametar *spi,
    uint8_t *buf, uint16_t len
);
Result spi_start_transceive_dma(
    SpiParametar *spi,
    uint8_t *tx_buf, uint8_t *rx_buf,
    uint16_t len
);


#endif