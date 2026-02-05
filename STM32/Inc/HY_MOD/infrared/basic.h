#pragma once
#include "main/config.h"
#ifdef HY_MOD_IR_TRSM

#define IR_CODE_LIST_MAX 20

// #define IR_NEC_LEADER_H 9000
// #define IR_NEC_LEADER_L 4500
// #define IR_NEC_MARK     562
// #define IR_NEC_SPACE_0  562
// #define IR_NEC_SPACE_1  1687

#define IR_NEC_LEADER_H 9000
#define IR_NEC_LEADER_L 4500
#define IR_NEC_MARK     562
#define IR_NEC_SPACE_0  562
#define IR_NEC_SPACE_1  1687

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
    IR_STATE_LEADER,
    IR_STATE_DATA,
    IR_STATE_END,
} IRState;

typedef struct IRCode
{
    uint8_t size;
    uint32_t data;
} IRCode;

typedef struct IRParameter
{
    const IRConst const_h;
    IRState state;
    IRCode  list[IR_CODE_LIST_MAX];
    uint8_t len;
    uint8_t byte;
    uint8_t bit;
} IRParameter;

extern IRParameter infrared_h;

void ir_code_gen(IRParameter *ir, IRCodes code);

#endif