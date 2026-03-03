#pragma once
#include "main/config.h"
#ifdef HY_MOD_STM32_FDCAN

#include "HY_MOD/main/typedef.h"
#include "HY_MOD/packet/fdcan.h"
#include "fdcan.h"

typedef enum FdcanState
{
    FDCAN_STATE_FINISH,
} FdcanState;

typedef struct FdcanConst
{
    FDCAN_HandleTypeDef *hfdcanx;
} FdcanConst;

typedef struct FdcanParametar
{
    const FdcanConst const_h;
    uint8_t task_tick;
    FdcanState state;
    bool bus_off;
    uint8_t rx_buf[FDCAN_PKT_LEN];
    FDCAN_RxHeaderTypeDef rx_header;
    FncState data_store;
    uint8_t trsming;
    uint8_t recving;
} FdcanParametar;

#endif