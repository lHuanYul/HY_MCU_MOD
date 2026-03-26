#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

const uint8_t hall_seq_ccw[8] = {UINT8_MAX, 5, 3, 1, 6, 4, 2, UINT8_MAX};
const uint8_t hall_seq_clw[8] = {UINT8_MAX, 3, 6, 2, 5, 1, 4, UINT8_MAX};

#endif