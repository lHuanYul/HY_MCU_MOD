#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"
#include "HY_MOD/main/typedef.h"

Result trigo_sin_cosf(float32_t angle, float32_t *sin, float32_t *cos);
Result trigo_atan(float32_t x, float32_t y, float32_t *theta);

#endif