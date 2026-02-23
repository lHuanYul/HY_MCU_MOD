#pragma once
#include "main/config.h"
#ifdef HY_MOD_LIGHT

#include "HY_MOD/main/typedef.h"

typedef struct LightConst
{
    GPIOData SER;
    GPIOData OE;
    GPIOData SRCLK;
    GPIOData RCLK;
} LightConst;

typedef struct LightSin
{
    const uint8_t id;
    const GPIOData gpio;
    uint16_t cd;
} LightSin;

#define LIGHT_BUTTON_CNT 3
typedef union LightAll
{
    struct {
        LightSin a;
        LightSin b;
        LightSin c;
    };
    LightSin abc[LIGHT_BUTTON_CNT];
} LightAll;

typedef struct LightCtrl
{
    const LightConst const_h;
    LightAll lights;
    uint8_t dbg_load;
    uint16_t cd;
    uint8_t load;
    uint8_t flags;
} LightCtrl;

#define LIGHT_OFF 0
#define LIGHT_ON 1
#define LIGHT_TOGGLE 2
#define LIGHT_CD 1200

void light_trigger(LightCtrl *light, uint8_t act, uint8_t id);

#endif