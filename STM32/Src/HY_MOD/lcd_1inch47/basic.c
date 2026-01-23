#include "HY_MOD/lcd_1inch47/basic.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

// 針對 1.47吋 (172x320) 的偏移量
// ST7789 原始解析度是 240寬，172寬的螢幕通常偏移 (240-172)/2 = 34
const LcdDatas waveshare_1inch47 = {
    .x_shift = 34,
    .y_shift = 0,
    .width = 172,
    .height = 320,
};

#include "spi.h"
#include "tim.h"

Lcd1I47Parametar lcd_1i47_h = {
    .const_h = {
        .lcd = &waveshare_1inch47,
        .spi_h = {
            .hspix = &hspi2,
            .MOSI = { GPIOD, GPIO_PIN_4 },
            .SCK  = { GPIOD, GPIO_PIN_1 },
            .NSS  = { GPIOD, GPIO_PIN_0 },
        },
        .DC  = { GPIOC, GPIO_PIN_10 },
        .RST = { GPIOC, GPIO_PIN_11 },
        .htimx = &htim14,
        .TIM_CHANNEL_x = TIM_CHANNEL_1,
    },
};

#define LCD_SPI lcd->const_h.spi_h.hspix

// 寫入命令
void lcd_write_cmd(Lcd1I47Parametar *lcd, uint8_t cmd)
{
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(LCD_SPI, &cmd, 1, 100);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_SET);
}

// 寫入資料
void lcd_write_data(Lcd1I47Parametar *lcd, uint8_t data)
{
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_RESET);
    HAL_SPI_Transmit(LCD_SPI, &data, 1, 100);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_SET);
}

// 寫入大量資料 (例如刷新整個畫面) - 使用 DMA 會更高效
void lcd_write_data_buf(Lcd1I47Parametar *lcd, uint8_t *buff, size_t size)
{
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_RESET);
    if (size >= 512)
    {
        HAL_SPI_Transmit_DMA(LCD_SPI, buff, size); // 或使用 HAL_SPI_Transmit_DMA
        while (HAL_SPI_GetState(lcd->const_h.spi_h.hspix) != HAL_SPI_STATE_READY);
        while (__HAL_SPI_GET_FLAG(lcd->const_h.spi_h.hspix, SPI_FLAG_BSY));
    }
    else HAL_SPI_Transmit(LCD_SPI, buff, size, 1000);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_SET);
}

#endif