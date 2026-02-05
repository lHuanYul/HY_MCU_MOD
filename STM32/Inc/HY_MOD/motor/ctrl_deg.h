#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

void deg_ctrl_120_load(MotorParameter *motor, uint8_t id);
void deg_ctrl_180_load(MotorParameter *motor);

#endif