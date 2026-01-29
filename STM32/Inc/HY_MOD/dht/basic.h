#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_DHT

#include "HY_MOD/main/typedef.h"

typedef struct DhtConst
{
    TIM_HandleTypeDef   *htimx;
    uint32_t            TIM_CHANNEL_x;
    HAL_TIM_ActiveChannel HAL_TIM_ACTIVE_CHANNEL_x;
    uint32_t            *tim_clk;
    GPIOData            gpio;
} DhtConst;

typedef enum
{
    DHT_STATE_FINISH,
    DHT_STATE_FINISHED,
    DHT_STATE_SEND,
    DHT_STATE_RESPONSE,
    DHT_STATE_DATA,
    DHT_STATE_ERROR,
} Dht11State;

typedef struct DhtParametar
{
    DhtConst    const_h;
    float32_t   dbg_tim_freq;
    uint32_t    last_cnt;
    bool        tim_mode_pwm;
    Dht11State  state;
    uint8_t     byte_x;
    uint8_t     bit_x;
    uint8_t     raw;
    // byte_0
    uint8_t     wet_i;
    // byte_1
    uint8_t     wet_d;
    // byte_2
    uint8_t     tmp_i;
    // byte_3
    uint8_t     tmp_d;
    // byte_4
    uint16_t    chk_sum;

    float32_t   wet;

    float32_t   tmp;
} DhtParametar;

void init_setup(DhtParametar *dht11);
void dht_tim_mode_switch(DhtParametar *dht11);

#endif