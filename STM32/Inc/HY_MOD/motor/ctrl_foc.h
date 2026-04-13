#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void motor_foc_set(MotorParameter *motor);
void motor_vec_ctrl_adcs_upd(MotorParameter *motor);
void motor_vec_ctrl_adcs_reset(MotorParameter *motor);
void motor_foc_run(MotorParameter *motor);
void motor_foc_load(MotorParameter *motor);

#endif