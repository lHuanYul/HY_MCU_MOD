#pragma once
#include "main/config.h"
#include "HY_MOD/main/basic.h"
#ifdef HY_MOD_STM32_LCD12864

#define HY_MOD_STM32_SPI
#include "HY_MOD/spi/basic.h"
#include "HY_MOD/lcd/graph.h"

/**
 * LCD12864
 * V0 電位越低對比越高
 * RS_CS    晶片致能 下拉
 * RW_SID   MOSI in Serial 下拉
 * EN_CLK   串列時脈 下拉
 * RST      上拉
 */

#define LCD12864_W 128
#define LCD12864_H 64
#define LCD12864_D 2
#define LCD_LINE_BYTES     (LCD12864_W / 8)          // 16 Bytes
#define LCD_DMA_LINE_LEN   (LCD_LINE_BYTES * 2 * 3)  // 32 Bytes * 3 = 96 Bytes

typedef struct Lcd12864Const
{
    SpiParametar    *spi_p;
    SpiCSConst      cs_set;
    GPIOData        RST;
} Lcd12864Const;

typedef enum LCD12864Mode
{
    LCD12864_UNINIT,
    LCD12864_INITING,
    LCD12864_READY,
    LCD12864_PREPARE,
    LCD12864_SENDING,
    LCD12864_WAITING,
} LCD12864Mode;

typedef union Curser
{
    struct {
        uint8_t x;
        uint8_t y;
    };
    uint8_t pos[2];
} Curser;

typedef struct Lcd12864Param
{
    const Lcd12864Const const_h;
    LCD12864Mode        mode;
    uint8_t             (*screen)[LCD12864_H][LCD12864_W / 8];
    uint8_t             (*dma_buf)[LCD_DMA_LINE_LEN];
    // 寫入游標
    Curser              curser_w;
    // 讀取游標
    Curser              curser_r;
} Lcd12864Param;

extern Lcd12864Param lcd12864_h;

/**
 * @brief 預設測試初始化：Layer 0 填 (0xFF, 0x00)，Layer 1 填 (0x00, 0xFF)
 */
void lcd12864_init_test_patterns(Lcd12864Param *lcd);
/**
 * @brief 將圖形寫入 LCD 顯示緩衝區，並保留 width 範圍外的原始資料
 * 
 * @param lcd    LCD 參數結構體指標
 * @param graph  圖形結構體指標
 * @param curser 游標/座標結構體指標
 */
Result lcd12864_display_write(Lcd12864Param *lcd, const LcdGraphData *graph, const Curser *curser);

#endif