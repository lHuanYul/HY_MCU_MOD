#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/typedef.h"
#include "HY_MOD/fdcan/packet.h"
#include "fdcan.h"

typedef enum FdcanState
{
    FDCAN_STATE_FINISH,
    FDCAN_STATE_BUS_OFF,
} FdcanState;

typedef struct FdcanConst
{
    FDCAN_HandleTypeDef *hfdcanx;
    TIM_HandleTypeDef   *htimx;
    uint32_t            *tim_clk;
} FdcanConst;

// DBG: debug
typedef struct FdcanDbg
{
    // 計時器頻率
    float32_t   tim_freq;
} FdcanDbg;

typedef struct FdcanParametar
{
    const FdcanConst const_h;
    FdcanDbg    dbg_h;
    FdcanPktPool pool;
    FdcanPktBuf tx_buf;
    uint8_t     tx_cb;
    uint32_t    tx_lost;
    FdcanPktBuf rx_buf;
    uint8_t     rx_cb;
    uint32_t    tim_tick;
    FdcanState  state;
    uint32_t    alive_tick;
#ifdef MCU_MOTOR_CTRL
    uint32_t        motor_alive;
    bool            motor_ret_en;
    volatile bool   motor_rpm_en;
    volatile bool   motor_idq_en1;
    volatile bool   motor_idq_en2;
#endif
} FdcanParametar;

#endif