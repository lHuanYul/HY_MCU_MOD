#include "HY_MOD/lcd/12864/basic.h"
#ifdef HY_MOD_STM32_LCD12864

#include "HY_MOD/main/buffer.h"

static ATTR_RAM_D2_ALIGN_32 uint8_t screen[LCD12864_D][LCD12864_H][LCD12864_W / 8];
static ATTR_RAM_D2_ALIGN_32 uint8_t line_dma_buf[1][LCD_DMA_LINE_LEN];

Lcd12864Param lcd12864_h =
{
    .const_h =
    {
        .spi_p  = &spi3_h,
        .cs_set =
        {
            .high_sel   = 1,
            .CS_NSS     = { GPIOD, GPIO_PIN_7 },
            .delay      = 100,
        },
        .RST    = { GPIOD, GPIO_PIN_5 },
    },
    .screen     = screen,
    .dma_buf    = line_dma_buf,
};

/**
 * @brief 將 screen 填入類似 Arduino FillGDRAM 的交錯字元圖樣
 * @note ST7920 的連續兩字元為 16-bit 單位 (data1 高 8 位，data2 低 8 位)
 */
static void lcd12864_fill_layer_gdram_pattern(Lcd12864Param *lcd, uint8_t layer, uint8_t data1, uint8_t data2)
{
    if (layer >= LCD12864_D) return;

    uint8_t (*target_screen)[LCD12864_W / 8] = lcd->screen[layer];

    for (uint8_t y = 0; y < LCD12864_H; y++)
    {
        // 寬度每 16 位元 (2 Bytes) 交替填入 data1 與 data2
        for (uint8_t x = 0; x < (LCD12864_W / 8); x += 2)
        {
            target_screen[y][x]     = data1;
            target_screen[y][x + 1] = data2;
        }
    }
}

/**
 * @brief 預設測試初始化：Layer 0 填 (0xFF, 0x00)，Layer 1 填 (0x00, 0xFF)
 */
void lcd12864_init_test_patterns(Lcd12864Param *lcd)
{
    lcd12864_fill_layer_gdram_pattern(lcd, 0, 0xFF, 0x00);
    lcd12864_fill_layer_gdram_pattern(lcd, 1, 0x00, 0xFF);
}

/**
 * @brief 將圖形寫入 LCD 顯示緩衝區，並保留 width 範圍外的原始資料
 * 
 * @param lcd    LCD 參數結構體指標
 * @param graph  圖形結構體指標
 * @param curser 游標/座標結構體指標
 */
Result lcd12864_display_write(Lcd12864Param *lcd, const LcdGraphData *graph, const Curser *curser)
{
    uint8_t z_depth = 0; // 預設寫入 layer 0
    lcd->curser_w = *curser;

    if (
        curser->x >= LCD12864_W || curser->y >= LCD12864_H ||
        curser->x + graph->width_bit > LCD12864_W || curser->y + graph->height > LCD12864_H
    ) return RESULT_ERROR(RESULT_ERROR_PARAM_OUT_OF_RANGE);

    // 強制轉型
    uint8_t *disp_ptr = (uint8_t *)lcd->screen;
    const uint8_t *data_ptr = (const uint8_t *)graph->data;

    // 預先計算位移參數
    uint8_t bit_offset = curser->x % 8;
    uint8_t byte_idx   = curser->x / 8;
    // 若 X 座標有細微位移，可能需要多寫入一個 Byte 來容納被向右擠出的像素
    uint8_t bytes_to_write = graph->width + (bit_offset > 0 ? 1 : 0);

    uint8_t i, j;
    for (i = 0; i < graph->height; i++)
    {
        for (j = 0; j < bytes_to_write; j++)
        {
            uint8_t current_col = byte_idx + j;
            if (current_col >= (LCD12864_W / 8)) break; // 螢幕右側硬邊界保護

            // 1. 取得滑動視窗內的圖形資料 (當前 Byte 與前一個 Byte)
            uint8_t curr_G = (j < graph->width) ? data_ptr[i * graph->width + j] : 0;
            uint8_t prev_G = (j > 0) ? data_ptr[i * graph->width + (j - 1)] : 0;

            // 2. 動態生成遮罩 (根據 width_bit 精準計算每個 Byte 的有效位元)
            uint8_t curr_M = 0, prev_M = 0;
            // 計算 curr_M 的有效遮罩
            if (graph->width_bit >= (j + 1) * 8) 
                curr_M = 0xFF;
            else if (graph->width_bit > j * 8)   
                curr_M = (uint8_t)(0xFF << (8 - (graph->width_bit % 8)));
            // 計算 prev_M 的有效遮罩
            if (j > 0)
            {
                if (graph->width_bit >= j * 8) 
                    prev_M = 0xFF;
                else if (graph->width_bit > (j - 1) * 8) 
                    prev_M = (uint8_t)(0xFF << (8 - (graph->width_bit % 8)));
            }

            // 3. 根據游標 X 軸位移 (bit_offset)，將兩個 Byte 拼接起來
            uint8_t data_to_write = 0;
            uint8_t mask_to_write = 0;

            if (bit_offset == 0)
            {
                // 無位移時直接賦值，避免 bit_offset 為 0 造成的位移 8 (未定義行為)
                data_to_write = curr_G;
                mask_to_write = curr_M;
            }
            else
            {
                // 將前一個 Byte 的尾巴與當前 Byte 的頭部拼接
                data_to_write = (uint8_t)(((prev_G << (8 - bit_offset)) | (curr_G >> bit_offset)) & 0xFF);
                mask_to_write = (uint8_t)(((prev_M << (8 - bit_offset)) | (curr_M >> bit_offset)) & 0xFF);
            }

            // 4. 計算實體記憶體偏移量並執行 Read-Modify-Write
            uint32_t offset = (z_depth * LCD12864_H * (LCD12864_W / 8)) +
                              ((curser->y + i) * (LCD12864_W / 8)) +
                              current_col;
            uint8_t original_data = disp_ptr[offset];
            // 將不屬於該圖形的區域清空 (利用 ~mask_to_write 保護原圖)，再填入新圖形
            disp_ptr[offset] = (original_data & ~mask_to_write) | (data_to_write & mask_to_write);
        }
    }

    return RESULT_OK(lcd);
}

#endif