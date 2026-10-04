#pragma once
#include "main/config.h"
#if defined(HY_MOD_STM32_LCD_1INCH47) || defined(HY_MOD_STM32_LCD12864)

#define HY_MOD_STM32_LCD

#include <stdint.h>

typedef struct LcdGraph8x16
{
    const uint8_t * const *datas;
    uint8_t  width;            // 共用寬度
    uint16_t width_bit;          // 共用實際寬度
    uint8_t  height;           // 共用高度
    uint8_t  offset;           // ASCII 偏移量 (例如 0x20)
} LcdGraph8x16;
extern const LcdGraph8x16 graph_8x16_const;

typedef struct LcdGraphData
{
    uint8_t     *data;
    uint8_t     width;
    uint16_t    width_bit;
    uint8_t     height;
} LcdGraphData;

typedef struct LcdGraphConst
{
    LcdGraphData    *datas;
    uint8_t         offset;
} LcdGraphConst;

typedef struct LcdGraphParm
{
    const LcdGraph8x16  *font_8x16;
    const LcdGraphData  *c_graphs;
} LcdGraphParm;

#endif
