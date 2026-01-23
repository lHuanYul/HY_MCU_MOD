#include "HY_MOD/lcd_1inch47/main.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

#include "HY_MOD/lcd_1inch47/font_8x16.h"

#define LCD_SPI lcd->const_h.spi_h.hspix

void LCD_init(Lcd1I47Parametar *lcd)
{
    uint8_t cmd;
    #define LCD_SPI_TRSM(x) \
    ({ \
        cmd = x; \
        HAL_SPI_Transmit(LCD_SPI, &cmd, 1, 100); \
    })

    HAL_TIM_PWM_Stop(lcd->const_h.htimx, lcd->const_h.TIM_CHANNEL_x);
    GPIO_WRITE(lcd->const_h.RST, GPIO_PIN_RESET);
    osDelay(100); // 保持低電位至少 10ms
    GPIO_WRITE(lcd->const_h.RST, GPIO_PIN_SET);
    osDelay(100); // 重置後等待晶片甦醒

    // 2. 喚醒 (Sleep Out)
    lcd_write_cmd(lcd, 0x11);
    osDelay(120);

    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_RESET);
    // 【新增】進入正常顯示模式 (Normal Display Mode On)
    // GPIO_WRITE(lcd_1i47->const_h.DC, GPIO_PIN_RESET);
    // LCD_SPI_TRSM(0x13);

    // 3. 記憶體資料存取控制 (MADCTL) - 控制方向
    // 0x00: 直向, 0x70: 橫向 (具體數值視你的螢幕貼裝方向而定)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0x36);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET); LCD_SPI_TRSM(0x00);

    // 4. 像素格式 (Color Mode)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0x3A);
    // 0x55 = 16-bit RGB565 (最常用)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET); LCD_SPI_TRSM(0x55);

    // ========================================================
    // Waveshare 1.47inch 官方修正參數
    // ========================================================
    
    // Porch Setting
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xB2);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    LCD_SPI_TRSM(0x0C); LCD_SPI_TRSM(0x0C); LCD_SPI_TRSM(0x00); LCD_SPI_TRSM(0x33); LCD_SPI_TRSM(0x33);

    // Gate Control
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xB7);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x35);

    // VCOM Setting (關鍵！0x19太亮，官方使用 0x37)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xBB);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x37); 

    // LCM Control
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xC0);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x2C);

    // VDV & VRH
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xC2);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x01);

    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xC3);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x12);

    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xC4);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x20);

    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xC6);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0x0F);

    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xD0);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);   LCD_SPI_TRSM(0xA4); LCD_SPI_TRSM(0xA1);

    // Gamma Positive (修正黑白問題)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xE0);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    LCD_SPI_TRSM(0xD0); LCD_SPI_TRSM(0x04); LCD_SPI_TRSM(0x0D); LCD_SPI_TRSM(0x11);
    LCD_SPI_TRSM(0x13); LCD_SPI_TRSM(0x2B); LCD_SPI_TRSM(0x3F); LCD_SPI_TRSM(0x54);
    LCD_SPI_TRSM(0x4C); LCD_SPI_TRSM(0x18); LCD_SPI_TRSM(0x0D); LCD_SPI_TRSM(0x0B);
    LCD_SPI_TRSM(0x1F); LCD_SPI_TRSM(0x23);

    // Gamma Negative
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0xE1);
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    LCD_SPI_TRSM(0xD0); LCD_SPI_TRSM(0x04); LCD_SPI_TRSM(0x0C); LCD_SPI_TRSM(0x11);
    LCD_SPI_TRSM(0x13); LCD_SPI_TRSM(0x2C); LCD_SPI_TRSM(0x3F); LCD_SPI_TRSM(0x44);
    LCD_SPI_TRSM(0x51); LCD_SPI_TRSM(0x2F); LCD_SPI_TRSM(0x1F); LCD_SPI_TRSM(0x1F);
    LCD_SPI_TRSM(0x20); LCD_SPI_TRSM(0x23);

    // 8. 反顯開關 (Inversion On)
    // IPS 螢幕通常需要開啟反顯，否則黑色會變白色
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0x21); 

    // 9. 開啟顯示 (Display On)
    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_RESET); LCD_SPI_TRSM(0x29);
    
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_SET);

    // 10. 開啟背光
    HAL_TIM_PWM_Start(lcd->const_h.htimx, lcd->const_h.TIM_CHANNEL_x);
    __HAL_TIM_SET_COMPARE(lcd->const_h.htimx, lcd->const_h.TIM_CHANNEL_x,
        (uint32_t)(lcd->const_h.htimx->Init.Period * 0.8));
}

void lcd_set_addr_window(Lcd1I47Parametar *lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4];

    // 加加上偏移量
    x1 += lcd->const_h.lcd->x_shift;
    x2 += lcd->const_h.lcd->x_shift;
    y1 += lcd->const_h.lcd->y_shift;
    y2 += lcd->const_h.lcd->y_shift;

    // 設定 Column Address (X軸)
    lcd_write_cmd(lcd, 0x2A); 
    data[0] = (x1 >> 8) & 0xFF;
    data[1] = x1 & 0xFF;
    data[2] = (x2 >> 8) & 0xFF;
    data[3] = x2 & 0xFF;
    // 這裡建議寫一個連續發送 Data 的函式，或者用 4次 Write_Data
    lcd_write_data_buf(lcd, data, 4);

    // 設定 Row Address (Y軸)
    lcd_write_cmd(lcd, 0x2B);
    data[0] = (y1 >> 8) & 0xFF;
    data[1] = y1 & 0xFF;
    data[2] = (y2 >> 8) & 0xFF;
    data[3] = y2 & 0xFF;
    lcd_write_data_buf(lcd, data, 4);

    // 準備開始寫入記憶體
    lcd_write_cmd(lcd, 0x2C);
}

static uint8_t lcd_dma_buffer[512];
void LCD_Fill_DMA(Lcd1I47Parametar *lcd, uint16_t color)
{
    uint32_t i;
    // 1. 先準備好「一行」的顏色數據
    uint8_t high_byte = (color >> 8) & 0xFF;
    uint8_t low_byte  = color & 0xFF;

    for (i = 0; i < lcd->const_h.lcd->width; i++)
    {
        lcd_dma_buffer[i * 2]     = high_byte;
        lcd_dma_buffer[i * 2 + 1] = low_byte;
    }

    // 2. 設定寫入視窗 (全螢幕)
    lcd_set_addr_window(lcd, 0, 0, lcd->const_h.lcd->width - 1, lcd->const_h.lcd->height - 1);

    GPIO_WRITE(lcd->const_h.DC, GPIO_PIN_SET);
    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_RESET);

    // 4. 分批傳送 (共 lcd->const_h.lcd->height 行)
    for (i = 0; i < lcd->const_h.lcd->height; i++)
    {
        HAL_SPI_Transmit_DMA(LCD_SPI, lcd_dma_buffer, 512); // 或使用 HAL_SPI_Transmit_DMA
        while (HAL_SPI_GetState(lcd->const_h.spi_h.hspix) != HAL_SPI_STATE_READY);
        while (__HAL_SPI_GET_FLAG(lcd->const_h.spi_h.hspix, SPI_FLAG_BSY));
    }

    GPIO_WRITE(lcd->const_h.spi_h.NSS, GPIO_PIN_SET);
}


// 繪製單個字符 (針對 Font0 結構優化)
void LCD_DrawChar(Lcd1I47Parametar *lcd, uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor)
{
    // 1. 範圍檢查 (僅支援 ASCII 32~126)
    if (c < ' ' || c > '~') return;

    // 2. 取得字型資訊
    // Font0.chars 是一個陣列，第0個元素對應 ASCII 32 (' ')
    const tChar *char_entry = &font_8x16.chars[c - ' ']; 
    const tImage *img = char_entry->image;

    // 雖然寬度固定是 8，但讀取 img->width 比較保險
    uint16_t w = img->width;  // 8
    uint16_t h = img->height; // 16
    const uint8_t *pdata = img->data;

    // 3. 邊界檢查
    if (x + w > lcd->const_h.lcd->width || y + h > lcd->const_h.lcd->height) return;

    // 4. 準備緩衝區 (8x16 = 128 pixels, 每個 pixel 2 bytes = 256 bytes)
    // 雖然是固定 8x16，但為了通用性，陣列開大一點無妨
    uint8_t buffer[256]; 
    uint16_t buf_idx = 0;

    // 5. 解析字型數據 (Top-to-Bottom, MSB First)
    // 每個 Byte 代表一行 (Row)，Bit 7 是最左邊的像素
    for (int row = 0; row < h; row++)
    {
        uint8_t line_data = pdata[row]; // 取出第 row 行的數據
        
        for (int col = 0; col < w; col++)
        {
            // 檢查 Bit (MSB First: Bit 7 -> Col 0)
            if ((line_data << col) & 0x80) 
            {
                // Bit = 1: 畫字體顏色
                buffer[buf_idx++] = (color >> 8) & 0xFF;
                buffer[buf_idx++] = color & 0xFF;
            }
            else
            {
                // Bit = 0: 畫背景顏色
                buffer[buf_idx++] = (bgcolor >> 8) & 0xFF;
                buffer[buf_idx++] = bgcolor & 0xFF;
            }
        }
    }

    // 6. 設定視窗並發送
    lcd_set_addr_window(lcd, x, y, x + w - 1, y + h - 1);
    lcd_write_data_buf(lcd, buffer, w * h * 2);
}

// 繪製字串 (自動換行)
void LCD_DrawString(Lcd1I47Parametar *lcd, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor)
{
    while (*str)
    {
        // 取得當前字寬 (Font0 固定為 8)
        uint8_t w = 8; 

        // 換行檢查
        if (x + w > lcd->const_h.lcd->width) {
            x = 0;
            y += 16; // 固定行高 16
        }
        
        // 邊界檢查 (超出螢幕底部就不畫了)
        if (y + 16 > lcd->const_h.lcd->height) break;

        LCD_DrawChar(lcd, x, y, *str, color, bgcolor);
        
        x += w; // 移動游標
        str++;
    }
}

#endif