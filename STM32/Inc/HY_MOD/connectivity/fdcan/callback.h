#pragma once
#include "HY_MOD/connectivity/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

void fdcan_error_status_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs);
void fdcan_tx_fifo_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs);
void fdcan_rx_fifo0_cb(
    FdcanParametar *fdcan,
    FDCAN_HandleTypeDef *hfdcan,
    uint32_t RxFifo0ITs,
    FdcanPktPool *pool,
    FdcanPktBuf *buf
);
void fdcan_rx_fifo1_cb(FdcanParametar *fdcan, FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs);

#endif
