#pragma once
#include "HY_MOD/lcd_1inch47/basic.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

void LCD_init(Lcd1I47Parametar *lcd);
void lcd_set_addr_window(Lcd1I47Parametar *lcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_Fill_DMA(Lcd1I47Parametar *lcd, uint16_t color);
void LCD_DrawChar(Lcd1I47Parametar *lcd, uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bgcolor);
void LCD_DrawString(Lcd1I47Parametar *lcd, uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor);

#endif