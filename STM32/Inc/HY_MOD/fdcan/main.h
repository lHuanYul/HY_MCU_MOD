#pragma once
#include "HY_MOD/fdcan/basic.h"
#ifdef HY_MOD_STM32_FDCAN

void fdcan_setup(FdcanParametar *fdcan);
void fdcan_tim_start(FdcanParametar *fdcan);
Result fdcan_tx_push(FdcanParametar *fdcan);
void fdcan_main(FdcanParametar *fdcan);

#endif