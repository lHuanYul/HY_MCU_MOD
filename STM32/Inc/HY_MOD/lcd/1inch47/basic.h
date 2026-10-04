#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_LCD_1INCH47

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

typedef struct LcdDatas
{
    uint16_t x_shift;
    uint16_t y_shift;
    uint16_t width;
    uint16_t height;
} LcdDatas;
extern const LcdDatas waveshare_1inch47;

typedef struct Lcd1I47Const
{
    SpiParametar    *spi_p;
    bool            high_sel;
    GPIOData        CS_NSS;
} Lcd1I47Const;

typedef struct Lcd1I47Const
{
    const LcdDatas *lcd;
    const Lcd1I47Const spi;
    GPIOData DC;
    GPIOData RST;
    TIM_HandleTypeDef   *htimx;
    uint32_t            TIM_CHANNEL_x;
} Lcd1I47Const;

typedef struct Lcd1I47Parametar
{
    const Lcd1I47Const const_h;
} Lcd1I47Parametar;

extern Lcd1I47Parametar lcd_1i47_h;

void lcd_write_cmd(Lcd1I47Parametar *lcd_1i47, uint8_t cmd);
void lcd_write_data(Lcd1I47Parametar *lcd_1i47, uint8_t data);
void lcd_write_data_buf(Lcd1I47Parametar *lcd_1i47, uint8_t *buff, size_t size);

#endif