#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

Result fdcan_pkt_transmit(FdcanPkt *pkt);
Result trsm_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf);
Result recv_pkts_proc(FdcanPktPool *pool, FdcanPktBuf *buf, uint8_t count);

#endif