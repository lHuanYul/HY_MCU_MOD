#pragma once
#include "main/config.h"
#include "HY_MOD/main/basic.h"
#ifdef HY_MOD_STM32_LCD12864

#define HY_MOD_STM32_SPI
#include "HY_MOD/spi/basic.h"

#define LCD12864_W 128
#define LCD12864_H 64

typedef struct Lcd12864Const
{
    SpiParametar spi_p;
} Lcd12864Const;

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
    uint8_t display[LCD12864_W / 8][LCD12864_H];
    // 寫入游標
    Curser curser_w;
    // 讀取游標
    Curser curser_r;
} Lcd12864Param;

#endif