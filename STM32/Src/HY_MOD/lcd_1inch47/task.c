#include "HY_MOD/lcd_1inch47/main.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

#include "main/lcd_1inch47.h"

#define LCD_TASK_DELAY_MS 1000
uint32_t hytest = 0;
void StartLcdTask(void *argument)
{
    Lcd1I47Parametar *lcd_1i47 = &lcd_1i47_h;
    // const uint32_t osPeriod = pdMS_TO_TICKS(LCD_TASK_DELAY_MS);
    // uint32_t next_wake = osKernelGetTickCount() + osPeriod;
    hytest++;
    LCD_init(lcd_1i47);
    for (;;)
    {
        hytest++;
        // LCD_Fill_DMA(lcd_1i47, 0xF800); // 刷成紅色
        // osDelay(1000);
        // LCD_Fill_DMA(lcd_1i47, 0x07E0); // 刷成綠色
        // osDelay(1000);
        // LCD_Fill_DMA(lcd_1i47, 0x001F); // 刷成藍色
        // osDelay(1000);
        LCD_Fill_DMA(lcd_1i47, 0x0000); // 清黑底

        // 顯示 "Hello" (白字黑底)
        LCD_DrawString(lcd_1i47, 10, 10, "Hello", 0xFFFF, 0x0000);

        // 顯示符號 (綠字黑底)
        LCD_DrawString(lcd_1i47, 10, 30, "!@#$%^&*", 0x07E0, 0x0000);

        osDelay(1000);
        // osDelayUntil(next_wake);
        // next_wake += osPeriod;
    }
}

#endif