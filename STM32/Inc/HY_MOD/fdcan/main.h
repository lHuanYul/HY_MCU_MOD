#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

void fdcan_setup(FdcanParametar *fdcan);
void fdcan_tim_start(FdcanParametar *fdcan);
Result fdcan_tx_push(FdcanParametar *fdcan, uint8_t cnt);
Result fdcan_trsm_pkts_proc(FdcanParametar *fdcan);
Result fdcan_recv_pkts_proc(FdcanParametar *fdcan, uint8_t count);

#endif