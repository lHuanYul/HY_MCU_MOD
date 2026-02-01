#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_YSM3

#include "HY_MOD/main/typedef.h"

typedef struct YSM3Const
{
    // 1
    GPIOData A1;
    // 2
    GPIOData A2;
    // 4
    GPIOData A3;
    // 8
    GPIOData A4;
    // 16
    GPIOData A5;
    // A10 GND BEFORE power
    GPIOData CN;
    // A11 STOP when GND
    GPIOData ST;
    GPIOData BY;
} YSM3Const;

typedef struct YSM3Parametar
{
    const YSM3Const const_h;
    uint8_t select;
} YSM3Parametar;

extern YSM3Parametar ysm3_h;

void ysm3_init(YSM3Parametar *ysm3);
void ysm3_load(YSM3Parametar *ysm3);

#endif