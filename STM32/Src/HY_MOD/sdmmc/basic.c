#include "HY_MOD/sdmmc/basic.h"
#ifdef HY_MOD_STM32_SDMMC
#include "sdmmc.h"
#include "mdma.h"

SD_CARD sd_card0 = {
    .const_h = {
        .hsdx = &hsd1,
        .hmdma = &hmdma_mdma_channel0_sdmmc1_end_data_0,
        .SDPath = SDPath,
    },
};

HAL_StatusTypeDef mdma_memcpy_align32(void *dst32, const void *src, size_t len)
{
    // 目標 dst32 請確保 __attribute__((aligned(32))) 或在 non-cacheable 區
    // 先搬「整數個 4 bytes」，尾端零星 <4 bytes 以 CPU 補尾
    size_t words = len / 4, rest = len % 4;

    if (words)
    {
        HAL_StatusTypeDef st = HAL_MDMA_Start(sd_card0.const_h.hmdma, (uint32_t)src, (uint32_t)dst32, words * 4, 1);
        if (st != HAL_OK) return st;
        st = HAL_MDMA_PollForTransfer(sd_card0.const_h.hmdma, HAL_MDMA_FULL_TRANSFER, 1000);
        if (st != HAL_OK) return st;
    }
    if (rest)
    {
        uint8_t *d = (uint8_t*)dst32 + words * 4;
        const uint8_t *s = (const uint8_t*)src + words * 4;
        while (rest--) *d++ = *s++;
    }
    return HAL_OK;
}

#endif