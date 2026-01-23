#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t dataSize;
} tImage;
typedef struct {
    long int code;
    const tImage *image;
} tChar;
typedef struct {
    int length;
    const tChar *chars;
} tFont;

extern const tFont font_8x16;

#endif