#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void motor_setup(MotorParameter *motor);
void motor_timer_load(MotorParameter *motor);

#endif