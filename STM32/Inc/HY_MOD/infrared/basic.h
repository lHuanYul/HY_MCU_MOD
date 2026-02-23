#pragma once
#include "main/config.h"
#ifdef HY_MOD_IR_TRSM

#define IR_CODE_LIST_MAX 100

typedef struct IRTime
{
    uint16_t header_mark;
    uint16_t header_space;
    uint16_t bit_mark;
    uint16_t bit_0;
    uint16_t bit_1;
} IRTime;

extern const IRTime ir_heran_fan;

#ifndef __HAL_TIM_SET_OCMODE
// Only for TIM17
#define __HAL_TIM_SET_OCMODE(HANDLE, CHANNEL, MODE) \
    do { \
        (HANDLE)->Instance->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1M_3); \
        (HANDLE)->Instance->CCMR1 |= (MODE); \
    } while(0)
#endif

typedef enum IRCodes
{
    IR_CODE_N,
    IR_CODE_HERAN_FAN_POWER,
    IR_CODE_HERAN_FAN_INCREASE,
    IR_CODE_HERAN_FAN_DECREASE,
} IRCodes;

typedef struct IRConst
{
    TIM_HandleTypeDef   *Master_htimx;
    uint32_t            *Master_tim_clk;
    uint32_t            Master_TIM_CHANNEL_x;
    TIM_HandleTypeDef   *Slave_htimx;
    uint32_t            Slave_TIM_CHANNEL_x;
} IRConst;

typedef enum IRState
{
    IR_STATE_IDLE,
    IR_STATE_RUN,
    IR_STATE_CD,
} IRState;

typedef struct IRParameter
{
    const IRConst const_h;
    IRState     state;
    uint8_t     cd;
    const IRTime  *time;
    uint32_t    data[IR_CODE_LIST_MAX];
    uint8_t     len;
    uint8_t     id;
} IRParameter;

void ir_code_gen(IRParameter *ir, IRCodes code);
uint16_t ir_code_time(uint8_t data);

#endif