#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

Result fdcan_pkt_transmit(FdcanParametar *fdcan);
Result fdcan_trsm_pkts_proc(FdcanParametar *fdcan, FdcanPktPool *pool, FdcanPktBuf *buf);
Result fdcan_recv_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf, uint8_t count);

#endif