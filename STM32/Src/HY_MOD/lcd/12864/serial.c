#include "HY_MOD/lcd/12864/main.h"
#ifdef HY_MOD_STM32_LCD12864

// ST7920 序列通訊微秒級延遲
static void lcd12864_delay_us(uint32_t us)
{
    // 以 CPU 頻率粗估或使用 DWT / 計時器延遲
    // 以 480MHz H7 為例，1 us 約 480 個 cycles
    for (volatile uint32_t i = 0; i < us * 17; i++) __NOP();
}

/**
 * @brief 寫入指令到 ST7920 (開頭前綴 0xF8)
 */
static Result lcd12864_write_command(Lcd12864Param *lcd, uint8_t cmd)
{
    // 使用 static 避免在 IT 非同步傳輸時 Stack 變數提早被釋放
    static uint8_t tx_buf[3];
    tx_buf[0] = 0xF8;               // 指令前綴 (RW=0, RS=0)
    tx_buf[1] = cmd & 0xF0;         // 傳送高 4 位
    tx_buf[2] = (cmd << 4) & 0xF0;  // 傳送低 4 位

    Result res = spi_start_transmit_it(lcd->const_h.spi_p, tx_buf, 3);
    lcd12864_delay_us(80);
    return res;
}

/**
 * @brief 寫入資料到 ST7920 (開頭前綴 0xFA)
 */
static Result lcd12864_write_data(Lcd12864Param *lcd, uint8_t data)
{
    static uint8_t tx_buf[3];
    tx_buf[0] = 0xFA;                // 資料前綴 (RW=0, RS=1)
    tx_buf[1] = data & 0xF0;         // 傳送高 4 位
    tx_buf[2] = (data << 4) & 0xF0;  // 傳送低 4 位

    Result res = spi_start_transmit_it(lcd->const_h.spi_p, tx_buf, 3);
    lcd12864_delay_us(80);
    return res;
}

Result lcd12864_serial_init(Lcd12864Param *lcd)
{
    lcd->mode = LCD12864_INITING;
    spi_init(lcd->const_h.spi_p);

    // 2. 執行硬體復位 (RST 低電平復位)
    spi_cs_unselect(&lcd->const_h.cs_set, lcd->const_h.cs_set.delay);
    GPIO_WRITE(lcd->const_h.RST, 0);
    osDelay(1);
    GPIO_WRITE(lcd->const_h.RST, 1);
    osDelay(1);
    spi_cs_select(&lcd->const_h.cs_set, lcd->const_h.cs_set.delay);

    // 3. 依據 ST7920 指令集進行軟體初始化
    // 0x30: 功能設定 (8-bit 介面、基本指令集)
    RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x30));

    // 0x0C: 顯示開關控制 (開顯示、關游標、關反白閃爍)
    RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x0C));

    // 0x01: 清除螢幕 (ST7920 執行清除需要較長處理時間 > 1.6ms)
    RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x01));
    osDelay(2);

    // 2. 移動游標至第一行第 1 個字 (0x80)
    RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x80));

    RESULT_CHECK_HANDLE(lcd12864_write_data(lcd, 'W'));
    RESULT_CHECK_HANDLE(lcd12864_write_data(lcd, 'O'));
    RESULT_CHECK_HANDLE(lcd12864_write_data(lcd, 'R'));
    RESULT_CHECK_HANDLE(lcd12864_write_data(lcd, 'L'));
    RESULT_CHECK_HANDLE(lcd12864_write_data(lcd, 'D'));
    
    spi_cs_unselect(&lcd->const_h.cs_set, lcd->const_h.cs_set.delay);
    lcd->mode = LCD12864_READY;// 1. 基本指令集 (0x30) 並開啟顯示 (0x0C)

    return RESULT_OK(lcd);
}

Result lcd12864_serial_flush_all(Lcd12864Param *lcd, uint8_t layer)
{
    if (layer >= LCD12864_D)
    {
        return RESULT_ERROR(RESULT_ERROR_PARAM_OUT_OF_RANGE);
    }
    uint8_t (*target_screen)[LCD_LINE_BYTES] = lcd->screen[layer];
    uint8_t *line_dma_buf = lcd->dma_buf[0];

    lcd->mode = LCD12864_SENDING;
    spi_cs_select(&lcd->const_h.cs_set, lcd->const_h.cs_set.delay);
    // 開啟繪圖 (RE=1, G=1)
    RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x36));

    // 2. 逐列設定座標並打包傳送 (共 32 條定址線)
    for (uint8_t y = 0; y < 32; y++)
    {
        // 設定垂直 Y 座標 (0x80 + y) 與 水平 X 座標 (0x80)
        RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x80 + y));
        RESULT_CHECK_HANDLE(lcd12864_write_command(lcd, 0x80));

        uint16_t buf_idx = 0;

        // 打包上半螢幕第 y 列像素 (16 Bytes -> 48 Bytes)
        for (uint8_t x = 0; x < LCD_LINE_BYTES; x++)
        {
            uint8_t pixel = target_screen[y][x];
            line_dma_buf[buf_idx++] = 0xFA;
            line_dma_buf[buf_idx++] = pixel & 0xF0;
            line_dma_buf[buf_idx++] = (pixel << 4) & 0xF0;
        }

        // 打包下半螢幕第 (y + 32) 列像素 (16 Bytes -> 48 Bytes)
        for (uint8_t x = 0; x < LCD_LINE_BYTES; x++)
        {
            uint8_t pixel = target_screen[y + 32][x];
            line_dma_buf[buf_idx++] = 0xFA;
            line_dma_buf[buf_idx++] = pixel & 0xF0;
            line_dma_buf[buf_idx++] = (pixel << 4) & 0xF0;
        }

        // 3. 透過 DMA
        RESULT_CHECK_HANDLE(spi_start_transmit_dma(
            lcd->const_h.spi_p,
            line_dma_buf, LCD_DMA_LINE_LEN
        ));
        lcd12864_delay_us(80);
    }

    spi_cs_unselect(&lcd->const_h.cs_set, lcd->const_h.cs_set.delay);
    lcd->mode = LCD12864_READY;

    return RESULT_OK(lcd);
}

#endif