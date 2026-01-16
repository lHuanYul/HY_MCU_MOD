#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_LIGHT

#include "HY_MOD/main/typedef.h"

typedef struct LightConst
{
    GPIOData SRCLK;
    GPIOData RCLK;
    GPIOData OE;
    GPIOData SER;
} LightConst;

typedef struct LightCtrl
{
    LightConst const_h;
    uint8_t data;
    const uint8_t light0;
    const uint8_t light1;
    const uint8_t light2;
    const uint8_t light3;
    const uint8_t light4;
    const uint8_t light5;
    const uint8_t light6;
    const uint8_t light7;
} LightCtrl;

#define LIGHT_ON(all,sel)       (all.data |= (sel))
#define LIGHT_OFF(all,sel)      (all.data &= ~(sel))
#define LIGHT_TOGGLE(all,sel)   (all.data ^= (sel))

#endif