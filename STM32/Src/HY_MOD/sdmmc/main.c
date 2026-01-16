#include "HY_MOD/sdmmc/main.h"
#ifdef HY_MOD_STM32_SDMMC
#include "HY_MOD/sdmmc/file.h"

#define CACHE_LINE_SIZE 32
#define ALIGN_UP(x) (((x) + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1))

ATTR_X(section(".dma_nc"), aligned(32)) uint8_t wbuf[8];
ATTR_X(section(".dma_nc"), aligned(32)) uint8_t rbuf[8] = {0};
static const char msg[] = "HELLO";

static bool sd_wait_transfer(uint32_t ms)
{
    uint32_t t = 0;
    while (HAL_SD_GetCardState(sd_card0.const_h.hsdx) != HAL_SD_CARD_TRANSFER) {
        osDelay(1);
        if (++t >= ms) return false;
    }
    return true;
}

uint32_t sd_running = 0;
static Result restart_check(SD_CARD *sd_card)
{
    if (sd_card->state == SD_CARD_STATE_READY) return RESULT_OK(sd_card);
    sd_running = 1;
    f_mount(NULL, sd_card->const_h.SDPath, 0);
    sd_card->hal_status = HAL_SD_Abort(sd_card->const_h.hsdx);
    sd_card->hal_status = HAL_SD_DeInit(sd_card->const_h.hsdx);
    FATFS_UnLinkDriver(sd_card->const_h.SDPath);
    sd_running = 2;
    sd_card->MSD_state = BSP_SD_Init();
    FATFS_LinkDriver(&SD_Driver, sd_card->const_h.SDPath);
    if (sd_card->MSD_state != MSD_OK) return RESULT_ERROR(RES_ERR_FAIL);
    uint32_t t=0;
    while (BSP_SD_GetCardState() != SD_TRANSFER_OK)
    {
        osDelay(1);
        if (++t >= 2000) return RESULT_ERROR(RES_ERR_FAIL);
    }
    sd_running = 3;
    sd_card->f_result = f_mount(&sd_card->fatfs_h, sd_card->const_h.SDPath, 1);
    if (sd_card->f_result != FR_OK) return RESULT_ERROR(RES_ERR_FAIL);
    sd_card->state = SD_CARD_STATE_READY;
    return RESULT_OK(sd_card);
}

#define SD_CARD_ACT(num, expr)              \
    ({                                      \
        sd_running = (num);                 \
        sd_card0.f_result = (expr);         \
        if (                                \
               sd_card0.f_result != FR_OK   \
        ) {                                 \
            sd_card0.state =                \
                SD_CARD_STATE_ERROR;        \
            osDelay(1000);                  \
            continue;                       \
        }                                   \
    })
void StartSDCardTask(void *argument)
{
    while (retSD != 0);
    for(;;)
    {
        if(RESULT_CHECK_RAW(restart_check(&sd_card0)))
        {
            osDelay(1000);
            continue;
        }
        SD_CARD_ACT(11, f_open(&sd_card0.file_h, file_test.const_h.name, FA_OPEN_ALWAYS | FA_WRITE));
        memcpy(wbuf, msg, sizeof(msg));
        SCB_CleanDCache_by_Addr((uint32_t*)wbuf, ALIGN_UP(sizeof(msg)));
        SD_CARD_ACT(12, f_write(&sd_card0.file_h, wbuf, sizeof(msg), &sd_card0.bits));
        /**
         * 如果f_sync錯誤code=2 CLK/CMD線注意
         */
        SD_CARD_ACT(13, f_sync(&sd_card0.file_h));
        SD_CARD_ACT(14, f_close(&sd_card0.file_h));
        osDelay(100);
        SD_CARD_ACT(21, f_open(&sd_card0.file_h, file_test.const_h.name, FA_OPEN_ALWAYS | FA_READ));
        SCB_InvalidateDCache_by_Addr((uint32_t*)rbuf, ALIGN_UP(sizeof(rbuf)));
        SD_CARD_ACT(22, f_read(&sd_card0.file_h, rbuf, sizeof(msg), &sd_card0.bits));
        SD_CARD_ACT(23, f_close(&sd_card0.file_h));
        sd_running = 100;
        osDelay(1000);
    }
}

#endif