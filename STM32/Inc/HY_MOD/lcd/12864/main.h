#pragma once
#include "HY_MOD/lcd/12864/basic.h"
#ifdef HY_MOD_STM32_LCD12864

Result lcd12864_serial_init(Lcd12864Param *lcd);

Result lcd12864_serial_flush_all(Lcd12864Param *lcd, uint8_t layer);

#endif