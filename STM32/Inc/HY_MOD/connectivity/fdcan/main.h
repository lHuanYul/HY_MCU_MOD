#pragma once
#include "HY_MOD/connectivity/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

Result fdcan_pkt_transmit(FdcanPkt *pkt);

#endif