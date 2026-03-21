#pragma once
#include "HY_MOD/motor/basic.h"
#ifdef HY_MOD_STM32_MOTOR

#include "HY_MOD/main/fn_state.h"

void motor_vec_ctrl_adcs_reset(MotorParameter *motor);
void motor_vec_ctrl_adcs_upd(MotorParameter *motor);
Result motor_vec_ctrl_angle_upd(MotorParameter *motor);
void motor_vec_ctrl_clarke(MotorParameter *motor);
void motor_vec_ctrl_park(MotorParameter *motor);
void motor_vec_ctrl_pi_id_iq(MotorParameter *motor);
void motor_vec_ctrl_ipark(MotorParameter *motor);
void motor_vec_ctrl_svgen(MotorParameter *motor);
void motor_vec_ctrl_svpwm(MotorParameter *motor);

#endif