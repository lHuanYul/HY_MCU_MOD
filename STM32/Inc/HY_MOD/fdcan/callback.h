#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

#define FDCAN_HAL_ErrorStatusCB_CALL(fdcan, hfdcan, ITs) \
    do { \
        if ( \
            INSTANCE_CHK((hfdcan), (fdcan).const_h.hfdcanx) \
        ) { \
            fdcan_error_status_cb(&(fdcan), (ITs)); \
        } \
    } while (0)
void fdcan_error_status_cb(FdcanParametar *fdcan, uint32_t ITs);

#define FDCAN_HAL_TxEventFifoCB_CALL(fdcan, hfdcan, ITs) \
    do { \
        if ( \
            INSTANCE_CHK((hfdcan), (fdcan).const_h.hfdcanx) \
        ) { \
            fdcan_tx_fifo_cb(&(fdcan), (ITs)); \
        } \
    } while (0)
void fdcan_tx_fifo_cb(FdcanParametar *fdcan, uint32_t ITs);

#define FDCAN_HAL_RxFifo0CB_CALL(fdcan, hfdcan, ITs) \
    do { \
        if ( \
            INSTANCE_CHK((hfdcan), (fdcan).const_h.hfdcanx) \
        ) { \
            fdcan_rx_fifo0_cb(&(fdcan), (ITs)); \
        } \
    } while (0)
void fdcan_rx_fifo0_cb(FdcanParametar *fdcan, uint32_t ITs);

#define FDCAN_HAL_RxFifo1CB_CALL(fdcan, hfdcan, ITs) \
    do { \
        if ( \
            INSTANCE_CHK((hfdcan), (fdcan).const_h.hfdcanx) \
        ) { \
            fdcan_rx_fifo1_cb(&(fdcan), (ITs)); \
        } \
    } while (0)
void fdcan_rx_fifo1_cb(FdcanParametar *fdcan, uint32_t ITs);

void fdcan_tim_cb(FdcanParametar *fdcan);

#endif
