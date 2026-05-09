#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void motor_setup(MotorParameter *motor);
void motor_timer_load(MotorParameter *motor);
// 從尾往轉子 順時針為負
void motor_set_spd(MotorParameter *motor, float32_t rpm);
void motor_set_rotate_mode(MotorParameter *motor, MotorRot mode);
void motor_switch_ctrl(MotorParameter *motor, MotorCtrl ctrl);
void motor_switch_ctrl_fix(MotorParameter *motor, MotorCtrl ctrl);
#endif